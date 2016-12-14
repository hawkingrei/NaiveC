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
                           }) {}

public:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;
    std::map<std::string, llvm::Value*> symbol_table;
    std::map<std::string, llvm::Type*> type_map;

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
};

Generator* Generator::_instance = nullptr;


#endif //NAIVEC_GENERATOR_H
