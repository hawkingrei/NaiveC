//
// Created by TianYi Zhuang on 14/12/2016.
//

#include "generator.h"
#include "parser.h"

static Generator* generator = Generator::instance();

llvm::Value* NumberExprAST::code_gen() {
    return llvm::ConstantInt::get(generator->context, llvm::APInt(32, (uint64_t) value, true));
}

llvm::Value* StrAST::code_gen() {
    auto str = llvm::ConstantDataArray::getString(generator->context, content.c_str());
    llvm::Type* type_p = generator->type_map["char"];
    type_p = llvm::ArrayType::get(type_p, content.size() + 1);

    llvm::AllocaInst* alloca = generator->builder.CreateAlloca(type_p);
    generator->builder.CreateStore(str, alloca);
    llvm::Value* value = generator->builder.CreateBitCast(alloca, llvm::Type::getInt8PtrTy(generator->context));

    return value;
}

llvm::Value* VariableExprAST::code_gen() {
    llvm::Value* v = Generator::instance()->symbol_table[name];
    // TODO: error
    return generator->builder.CreateLoad(v, name);
}

llvm::Value* BinaryExprAST::code_gen() {
    llvm::Value* l = left->code_gen();
    llvm::Value* r = right->code_gen();

    if (!l || !r) {
        return nullptr;
    }

    switch (op) {
        case '+':
            return generator->builder.CreateAdd(l, r, "addtmp");
        case '-':
            return generator->builder.CreateSub(l, r, "subtmp");
        case '*':
            return generator->builder.CreateMul(l, r, "multmp");
        case '<':
            l = generator->builder.CreateICmpULT(l, r, "cmptmp");
            return generator->builder.CreateSExt(
                l, llvm::Type::getInt32Ty(generator->context), "booltmp");
        default:
            return nullptr;
    }
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
    std::vector<llvm::Type*> values(args.size(), llvm::Type::getInt32Ty(generator->context));
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

    llvm::Type* type_p = generator->type_map[type];
    if (is_array) {
        type_p = llvm::ArrayType::get(type_p, array_length);
    }

    llvm::AllocaInst* var = generator->create_entry_block_alloca(f, type_p, name);
    generator->builder.CreateStore(llvm::ConstantInt::get(generator->type_map["int"], 0, true), var);
    generator->symbol_table[name] = var;

    return var;
}

llvm::Value* AssignStatementAST::code_gen() {
    return generator->builder.CreateStore(expr_r->code_gen(), generator->symbol_table[var_name]);
}

llvm::Value* ReturnStatementAST::code_gen() {
    return generator->builder.CreateRet(expr->code_gen());
}

llvm::Value* IfStatementAST::code_gen() {
    llvm::Value* cond_v = cond->code_gen();

    if (!cond_v) {
        return nullptr;
    }

    cond_v = generator->builder.CreateICmpNE(
        cond_v, llvm::ConstantInt::get(llvm::Type::getInt32Ty(generator->context), 0, true), "ifcond");

    llvm::Function* function = generator->builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* if_basic_block = llvm::BasicBlock::Create(generator->context, "if", function);
    llvm::BasicBlock* else_basic_block = llvm::BasicBlock::Create(generator->context, "else", function);
    llvm::BasicBlock* merge_basic_block = llvm::BasicBlock::Create(generator->context, "ifcont", function);

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
    llvm::Value *start_v = start->code_gen();
    if (!start_v)
        return nullptr;

    generator->symbol_table[var_name];

    llvm::Function *function = generator->builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *loop_start_b =
            llvm::BasicBlock::Create(generator->context, "loopstart", function);
    llvm::BasicBlock *loop_body_b =
            llvm::BasicBlock::Create(generator->context, "loopbody", function);
    llvm::BasicBlock *loop_end_b =
            llvm::BasicBlock::Create(generator->context, "loopend", function);

    generator->builder.CreateBr(loop_start_b);

    generator->builder.SetInsertPoint(loop_start_b);
    llvm::Value *cond_v = cond->code_gen();
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

    llvm::BasicBlock* block = llvm::BasicBlock::Create(generator->context, "entry", function);
    generator->builder.SetInsertPoint(block);

    generator->symbol_table.clear();

    for (auto& arg: function->args()) {
        llvm::AllocaInst* alloca = generator->create_entry_block_alloca(
            function, generator->type_map["int"], proto->get_name());
        generator->builder.CreateStore(&arg, alloca);

        generator->symbol_table[arg.getName()] = alloca;
    }

    for (const auto& stat: body) {
        stat->code_gen();
    }

    llvm::verifyFunction(*function);
    return function;
}
