//
// Created by TianYi Zhuang on 14/12/2016.
//

#include "generator.h"
#include "parser.h"

Generator* Generator::_instance = nullptr;

static Generator* generator = Generator::instance();

void Generator::extern_printf() {
    std::vector<llvm::Type*> puts_args;
    puts_args.push_back(builder.getInt8Ty()->getPointerTo());
    llvm::ArrayRef<llvm::Type*> args_ref(puts_args);

    llvm::FunctionType* puts_type =
        llvm::FunctionType::get(builder.getInt32Ty(), args_ref, true);

    llvm::Function* function =
        llvm::Function::Create(
            puts_type,
            llvm::Function::ExternalLinkage,
            "printf",
            module.get()
        );

    function->dump();
}

void Generator::extern_gets() {
    std::vector<llvm::Type*> gets_args;
    gets_args.push_back(builder.getInt8PtrTy());
    llvm::ArrayRef<llvm::Type*> args_ref(gets_args);
    llvm::FunctionType* gets_type =
        llvm::FunctionType::get(builder.getInt8PtrTy(), args_ref, false);

    llvm::Function* function =
        llvm::Function::Create(
            gets_type,
            llvm::Function::ExternalLinkage,
            "gets",
            module.get()
        );

    function->dump();
}

void Generator::dump(std::string filename) {
    std::error_code ec;
    llvm::raw_fd_ostream os(filename, ec, llvm::sys::fs::F_None);
    llvm::WriteBitcodeToFile(module.get(), os);
}

llvm::Value* NumberExprAST::code_gen() {
    return llvm::ConstantInt::get(generator->context, llvm::APInt(32, (uint64_t) value, true));
}

llvm::Value* StrAST::code_gen() {
    auto str = llvm::ConstantDataArray::getString(generator->context, content.c_str());
    llvm::Type* type_p = generator->type_map["char"];
    type_p = llvm::ArrayType::get(type_p, content.size() + 1);

    llvm::AllocaInst* alloca = generator->builder.CreateAlloca(type_p, nullptr,
                                                               "var" + std::to_string(generator->label_id));
    ++generator->label_id;
    generator->builder.CreateStore(str, alloca);
    llvm::Value* value = generator->builder.CreateBitCast(alloca, llvm::Type::getInt8PtrTy(generator->context));

    return value;
}

llvm::Value* CharAST::code_gen() {
    return llvm::ConstantInt::get(generator->context, llvm::APInt(8, value, true));
}

llvm::Value* VariableExprAST::code_gen() {
    llvm::Value* v = Generator::instance()->symbol_table[name];

    if (is_array) {
        llvm::Value* index_v = index->code_gen();
        llvm::Value* gep_v = get_ptr_of_value(v, index_v);
        return generator->builder.CreateLoad(gep_v);
    } else {
        if (generator->symbol_is_ptr_table[name]) {
            llvm::Value* zero = llvm::ConstantInt::get(generator->context, llvm::APInt(32, 0));
            return get_ptr_of_value(v, zero);
        } else {
            llvm::Value* v = Generator::instance()->symbol_table[name];
            return generator->builder.CreateLoad(v, name);
        }
    }
}

llvm::Value* VariableExprAST::get_ref() {
    if (is_array) {
        llvm::Value* v = Generator::instance()->symbol_table[name];
        llvm::Value* index_v = index->code_gen();
        return get_ptr_of_value(v, index_v);
    } else {
        return Generator::instance()->symbol_table[name];
    }
}

llvm::Value* VariableExprAST::get_ptr_of_value(llvm::Value* v, llvm::Value* index) {
    if (generator->symbol_type_table[name] == parser::POINTER) {
        llvm::Value* point_v = generator->builder.CreateLoad(v);
        return generator->builder.CreateGEP(point_v, index);
    }
    std::vector<llvm::Value*> indexes;
    llvm::Value* zero = llvm::ConstantInt::get(generator->context, llvm::APInt(32, 0));
    indexes.push_back(zero);
    indexes.push_back(index);
    return generator->builder.CreateGEP(v, indexes);
}

llvm::Value* BinaryExprAST::code_gen() {
    llvm::Value* l = left->code_gen();
    llvm::Value* r = right->code_gen();

    if (!l || !r) {
        return nullptr;
    }

    if (op == "+") {
        return generator->builder.CreateAdd(l, r, "addtmp");
    } else if (op == "-") {
        l = generator->builder.CreateSub(l, r, "subtmp");
        return generator->builder.CreateZExt(l, generator->builder.getInt32Ty(), "intsubtmp");
    } else if (op == "*") {
        return generator->builder.CreateMul(l, r, "multmp");
    } else if (op == "/") {
        return generator->builder.CreateSDiv(l, r, "divtmp");
    } else if (op == "<") {
        l = generator->builder.CreateICmpSLT(l, r, "cmplttmp");
        return generator->builder.CreateZExt(l, generator->builder.getInt32Ty(), "boollttmp");
    } else if (op == "==") {
        l = generator->builder.CreateICmpEQ(l, r, "cmpeqtmp");
        return generator->builder.CreateZExt(l, generator->builder.getInt32Ty(), "booletmp");
    } else if (op == "!=") {
        l = generator->builder.CreateICmpNE(l, r, "cmpnetmp");
        return generator->builder.CreateZExt(l, generator->builder.getInt32Ty(), "boolnetmp");
    }

    return nullptr;
}

llvm::Value* CallExprAST::code_gen() {
    llvm::Function* callee_f = generator->module->getFunction(callee);

    if (!callee_f) {
        throw std::exception();
    }

    std::vector<llvm::Value*> args_v;
    for (const auto& arg: args) {
        args_v.push_back(arg->code_gen());
        if (!args_v.back()) {
            return nullptr;
        }
    }

    return generator->builder.CreateCall(callee_f, args_v, "calltmp");
}

llvm::Function* PrototypeAST::code_gen() {
//    std::vector<llvm::Type*> values(args.size(), llvm::Type::getInt32Ty(generator->context));
    std::vector<llvm::Type*> values;
    for (auto& arg_type: arg_types) {
        std::string type_str;
        if (arg_type->form == parser::RAW_TYPE) {
            type_str = arg_type->type.type_name;
        } else {
            type_str = arg_type->type.raw_type->type.type_name + "ptr";
        }
        values.push_back(generator->type_map[type_str]);
    }

    llvm::FunctionType* ft = llvm::FunctionType::get(generator->type_map[ret_type], values, false);
    llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, generator->module.get());

    unsigned idx = 0;
    for (auto& arg: f->args()) {
        arg.setName(args[idx++]);
    }

    return f;
}

llvm::Value* DeclareStatementAST::code_gen() {
    llvm::Function* f = generator->builder.GetInsertBlock()->getParent();
    llvm::Type* llvm_type_p;

    if (type_p->form == parser::RAW_TYPE) {
        llvm_type_p = generator->type_map[type_p->type.type_name];
    } else if (type_p->form == parser::ARRAY) {
        // array
        llvm_type_p = generator->type_map[type_p->type.raw_type->type.type_name];
        llvm_type_p = llvm::ArrayType::get(llvm_type_p, type_p->type.length);
    } else {
        llvm_type_p = generator->type_map[type_p->type.raw_type->type.type_name + "ptr"];
    }

    llvm::AllocaInst* var = nullptr;

    var = generator->create_entry_block_alloca(f, llvm_type_p, var_name);
    if (type_p->form == parser::RAW_TYPE) {
        generator->builder.CreateStore(llvm::ConstantInt::get(generator->type_map[type_p->type.type_name], 0, true), var);
    }
    generator->symbol_table[var_name] = var;
    generator->symbol_is_ptr_table[var_name] = type_p->form;

    return var;
}

llvm::Value* AssignStatementAST::code_gen() {
    return generator->builder.CreateStore(expr_r->code_gen(), expr_l->get_ref());
}

llvm::Value* ReturnStatementAST::code_gen() {
    return generator->builder.CreateRet(expr->code_gen());
}

llvm::Value* CallStatement::code_gen() {
    return expr->code_gen();
}

llvm::Value* IfStatementAST::code_gen() {
    llvm::Value* cond_v = cond->code_gen();

    if (!cond_v) {
        return nullptr;
    }

    cond_v = generator->builder.CreateICmpNE(
        cond_v, llvm::ConstantInt::get(llvm::Type::getInt32Ty(generator->context), 0, true),
        "condcmp" + std::to_string(generator->label_id));

    llvm::Function* function = generator->builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* if_basic_block = llvm::BasicBlock::Create(generator->context, "", function);
    llvm::BasicBlock* else_basic_block = llvm::BasicBlock::Create(generator->context, "", function);
    llvm::BasicBlock* merge_basic_block = llvm::BasicBlock::Create(generator->context, "", function);

    generator->builder.CreateCondBr(cond_v, if_basic_block, else_basic_block);
    generator->builder.SetInsertPoint(if_basic_block);

    std::vector<llvm::Value*> if_values;

    for (const auto& stat: if_block->statements) {
        if_values.push_back(stat->code_gen());
    }

    generator->builder.CreateBr(merge_basic_block);

    function->getBasicBlockList().push_back(else_basic_block);
    generator->builder.SetInsertPoint(else_basic_block);

    std::vector<llvm::Value*> else_values;
    if (else_block) {
        for (const auto& stat: else_block->statements) {
            else_values.push_back(stat->code_gen());
        }
    }

    generator->builder.CreateBr(merge_basic_block);

    function->getBasicBlockList().push_back(merge_basic_block);
    generator->builder.SetInsertPoint(merge_basic_block);

    return nullptr;
}

llvm::Value* ForStatementAST::code_gen() {
    llvm::Value* start_v = start->code_gen();
    if (!start_v) {
        return nullptr;
    }

//    generator->symbol_table[var_name];

    llvm::Function* function = generator->builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* loop_start_b =
        llvm::BasicBlock::Create(generator->context, "", function);
    llvm::BasicBlock* loop_body_b =
        llvm::BasicBlock::Create(generator->context, "", function);
    llvm::BasicBlock* loop_end_b =
        llvm::BasicBlock::Create(generator->context, "", function);

    generator->builder.CreateBr(loop_start_b);

    generator->builder.SetInsertPoint(loop_start_b);
    llvm::Value* cond_v = cond->code_gen();
    cond_v = generator->builder.CreateICmpNE(
        cond_v, llvm::ConstantInt::get(llvm::Type::getInt32Ty(generator->context), 0, true), "loopcond");
    generator->builder.CreateCondBr(cond_v, loop_body_b, loop_end_b);

    generator->builder.SetInsertPoint(loop_body_b);
    if (for_block) {
        for (const auto& stat: for_block->statements) {
            stat->code_gen();
        }
    }
    step->code_gen();
    generator->builder.CreateBr(loop_start_b);
    generator->builder.SetInsertPoint(loop_end_b);
    return nullptr;
}

llvm::BasicBlock* CodeBlockAST::code_gen() {
    llvm::BasicBlock* old_block = generator->builder.GetInsertBlock();
    llvm::Function* function = old_block->getParent();
    llvm::BasicBlock* block = llvm::BasicBlock::Create(generator->context, "b", function);
    generator->builder.SetInsertPoint(block);
    for (const auto& stat : statements) {
        stat->code_gen();
    }
    generator->builder.SetInsertPoint(old_block);
    return block;
}

llvm::Function* FunctionAST::code_gen() {
    llvm::Function* function = generator->module->getFunction(proto->get_name());
    if (!function) {
        function = proto->code_gen();
    }

    if (!function) {
        return nullptr;
    }

    llvm::BasicBlock* block = llvm::BasicBlock::Create(generator->context,
                                                       "entry" + std::to_string(generator->label_id), function);

    ++generator->label_id;

    generator->builder.SetInsertPoint(block);

//    generator->symbol_table.clear();
    size_t idx = 0;
    for (auto& arg: function->args()) {
        auto& arg_type = proto->arg_types[idx];
        std::string type_str;
        if (arg_type->form == parser::RAW_TYPE) {
            type_str = arg_type->type.type_name;
        } else {
            type_str = arg_type->type.raw_type->type.type_name + "ptr";
        }
        ++idx;
        llvm::Type* type_v = generator->type_map[type_str];

        llvm::AllocaInst* alloca = generator->create_entry_block_alloca(
            function, type_v, proto->get_name());
        generator->builder.CreateStore(&arg, alloca);

        generator->symbol_table[arg.getName()] = alloca;
        generator->symbol_type_table[arg.getName()] = arg_type->form;
    }

    for (const auto& stat: body) {
        stat->code_gen();
    }

    llvm::verifyFunction(*function);
    return function;
}
