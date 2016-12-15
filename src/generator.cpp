//
// Created by TianYi Zhuang on 14/12/2016.
//

#include "generator.h"
#include "parser.h"

static Generator* generator = Generator::instance();

llvm::Value* NumberExprAST::code_gen() {
    return llvm::ConstantInt::get(Generator::instance()->context, llvm::APInt(32, (uint64_t) value, true));
}

llvm::Value* VariableExprAST::code_gen() {
    llvm::Value* v = Generator::instance()->symbol_table[name];
    // TODO: error
    return v;
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
            return generator->builder.CreateUIToFP(
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
    llvm::Type* type_p = generator->type_map[type];
    if (is_array) {
        type_p = llvm::ArrayType::get(type_p, array_length);
    }

    llvm::Value* var;

    if (is_global) {
        var = new llvm::GlobalVariable(
            *generator->module, type_p, false,
            llvm::GlobalValue::ExternalLinkage, 0, name);
    } else {
        var = generator->builder.CreateAlloca(
            type_p, nullptr, name);
    }

    generator->symbol_table[name] = var;

    return var;
}

llvm::Value* AssignStatementAST::code_gen() {
    return generator->builder.CreateStore(expr_r->code_gen(), expr_l->code_gen());
}

llvm::Value* ReturnStatementAST::code_gen() {
    return generator->builder.CreateRet(expr->code_gen());
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
        generator->symbol_table[arg.getName()] = &arg;
    }

    for (const auto& stat: body) {
        stat->code_gen();
    }

    llvm::verifyFunction(*function);
    return function;
}
