//
// Created by TianYi Zhuang on 14/12/2016.
//

#ifndef NAIVEC_GENERATOR_H
#define NAIVEC_GENERATOR_H

#include <memory>
#include <map>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>


class Generator {
private:
    static Generator* _instance;

    Generator() : builder(context),
                  module(std::make_unique<llvm::Module>("jit", context)),
                  type_map({{"int",  llvm::Type::getInt32Ty(context)},
                            {"char", llvm::Type::getInt8Ty(context)},
                           }) {

        std::vector<llvm::Type *> puts_args;
        puts_args.push_back(builder.getInt8Ty()->getPointerTo());
        llvm::ArrayRef<llvm::Type*>  argsRef(puts_args);

        llvm::FunctionType *puts_type =
            llvm::FunctionType::get(builder.getInt32Ty(), argsRef, true);

        llvm::Function* function = llvm::Function::Create(puts_type, llvm::Function::ExternalLinkage, "printf", module.get());
        function->dump();
    }

public:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    llvm::Constant* puts_func;
    std::unique_ptr<llvm::Module> module;
    std::map<std::string, llvm::AllocaInst*> symbol_table;
    std::map<std::string, bool> symbol_flag;
    std::map<std::string, llvm::Type*> type_map;

    std::vector<llvm::Value *> constants;

    Generator(const Generator&) = delete;

    Generator(Generator&&) = delete;

    ~Generator() {
        module->dump();
    }

    inline static Generator* instance() {
        if (!_instance) {
            _instance = new Generator();
        }

        return _instance;
    }

    llvm::AllocaInst* create_entry_block_alloca(
        llvm::Function* f, llvm::Type* type_p, const std::string& var_name) {
        llvm::IRBuilder<> tmp_builder(&f->getEntryBlock(), f->getEntryBlock().begin());
        return tmp_builder.CreateAlloca(type_p, nullptr, var_name);
    }
};

Generator* Generator::_instance = nullptr;


#endif //NAIVEC_GENERATOR_H
