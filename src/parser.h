//
// Created by TianYi Zhuang on 08/12/2016.
//

#ifndef NAIVEC_PARSER_H
#define NAIVEC_PARSER_H

#include <string>
#include <vector>
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
#include "lexer.h"

class ExprAST {
public:
    virtual llvm::Value* code_gen() = 0;

    virtual ~ExprAST() {}
};

class NumberExprAST : public ExprAST {
private:
    int value;
public:
    NumberExprAST(int value) : value(value) {}

    virtual llvm::Value* code_gen() override;
};

class VariableExprAST : public ExprAST {
private:
    std::string name;
public:
    VariableExprAST(std::string name) : name(name) {}

    virtual llvm::Value* code_gen() override;
};

class BinaryExprAST : public ExprAST {
private:
    char op;
    std::unique_ptr<ExprAST> left;
    std::unique_ptr<ExprAST> right;
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST>&& left, std::unique_ptr<ExprAST>&& right)
        : op(op), left(std::move(left)), right(std::move(right)) {}

    virtual llvm::Value* code_gen() override;
};

class CallExprAST : public ExprAST {
private:
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;
public:
    CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>>&& args)
        : callee(callee), args(std::move(args)) {}

    virtual llvm::Value* code_gen() override;
};

class DeclareAST {
private:
    std::string type;
    std::string name;
    bool is_array;
    size_t array_length;
    bool is_global;
public:
    DeclareAST(std::string type, const std::string& name, bool is_array = false,
               size_t array_length = 0, bool is_global = false)
        : type(type), name(name), is_array(is_array),
          array_length(array_length), is_global(is_global) {}

    inline void set_global(const bool g) {
        is_global = g;
    }

    llvm::Value* code_gen();
};

class PrototypeAST {
private:
    std::string ret_type;
    std::string name;
    std::vector<std::string> args;

public:
    PrototypeAST(const std::string& ret_type, const std::string& name,
                 std::vector<std::string> args)
        : ret_type(ret_type), name(name), args(args) {}

    llvm::Function* code_gen();

    const std::string& get_name() const {
        return name;
    }
};

class FunctionAST {
    std::unique_ptr<PrototypeAST> proto;
    std::unique_ptr<ExprAST> body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST>&& proto, std::unique_ptr<ExprAST>&& body)
        : proto(std::move(proto)), body(std::move(body)) {}

    llvm::Function* code_gen();
};


class Parser {
private:
    std::vector<Token> tokens;
    std::vector<Token>::iterator token_it;
    static std::map<char, int> precedence_map;

    int get_token_prec();

    void assert_token(TokenType token_type) throw();

public:

    std::unique_ptr<ExprAST> parse_expr();

    std::unique_ptr<ExprAST> parse_number_expr();

    std::unique_ptr<ExprAST> parse_paren_expr();

    std::unique_ptr<ExprAST> parse_identifier_expr();

    std::unique_ptr<ExprAST> parse_primary();

    std::unique_ptr<ExprAST> parse_bin_op_right(int, std::unique_ptr<ExprAST>);

    std::unique_ptr<DeclareAST> parse_declare();

    std::unique_ptr<PrototypeAST> parse_prototype();

    std::unique_ptr<FunctionAST> parse_definition();

    void handle_definition();

    void handle_global_declare();

    void main_loop();

    Parser(const std::vector<Token>& tokens)
        : tokens(tokens), token_it(this->tokens.begin()) {}
};


#endif //NAIVEC_PARSER_H
