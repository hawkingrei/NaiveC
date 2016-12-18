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
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/FileSystem.h>
#include "parser.h"


class Generator {
private:
    static Generator* _instance;

    Generator() : builder(context),
                  module(std::make_unique<llvm::Module>("jit", context)),
                  type_map({{"int",  llvm::Type::getInt32Ty(context)},
                            {"char", llvm::Type::getInt8Ty(context)},
                            {"intptr", llvm::Type::getInt32PtrTy(context)},
                            {"charptr", llvm::Type::getInt8PtrTy(context)}
                           }) {
        extern_printf();
        extern_gets();
    }

    void extern_printf();

    void extern_gets();

public:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    llvm::Constant* puts_func;
    std::unique_ptr<llvm::Module> module;
    std::map<std::string, llvm::AllocaInst*> symbol_table;
    std::map<std::string, llvm::Type*> type_map;
    std::map<std::string, bool> symbol_is_ptr_table;
    std::map<std::string, parser::TypeForm> symbol_type_table;

    size_t label_id = 0;

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

    void dump(std::string filename);
};


#endif //NAIVEC_GENERATOR_H
