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

class CodeBlockAST;

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

class StrAST : public ExprAST {
private:
    std::string content;
public:
    StrAST(const std::string& content)
        : content(content) {}

    virtual llvm::Value* code_gen() override;
};

class VariableExprAST : public ExprAST {
private:
    std::string name;
    bool is_array;
    std::unique_ptr<ExprAST> index;
public:
    VariableExprAST(std::string name, bool is_array = false, std::unique_ptr<ExprAST> index = nullptr) :
            name(name), is_array(is_array), index(std::move(index)) {}
    llvm::Value* get_ref();
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

class StatementAST {
public:
    virtual ~StatementAST() {}

    virtual llvm::Value* code_gen() = 0;
};

class AssignStatementAST : public StatementAST {
private:
    std::unique_ptr<VariableExprAST> expr_l;
    std::unique_ptr<ExprAST> expr_r;

public:
    AssignStatementAST(std::unique_ptr<VariableExprAST>&& expr_l, std::unique_ptr<ExprAST>&& expr_r)
        : expr_l(std::move(expr_l)), expr_r(std::move(expr_r)) {}

    virtual llvm::Value* code_gen() override;
};

class DeclareStatementAST : public StatementAST {
private:
    std::string type;
    std::string name;
    bool is_array;
    size_t array_length;
    bool is_global;
public:
    DeclareStatementAST(std::string type, const std::string& name, bool is_array = false,
                        size_t array_length = 0, bool is_global = false)
        : type(type), name(name), is_array(is_array),
          array_length(array_length), is_global(is_global) {}

    inline void set_global(const bool g) {
        is_global = g;
    }

    virtual llvm::Value* code_gen() override;
};

class ReturnStatementAST : public StatementAST {
private:
    std::unique_ptr<ExprAST> expr;
public:
    ReturnStatementAST(std::unique_ptr<ExprAST>&& expr)
        : expr(std::move(expr)) {}

    virtual llvm::Value* code_gen() override;
};

class CallStatement : public StatementAST {
private:
    std::unique_ptr<ExprAST> expr;

public:
    CallStatement(std::unique_ptr<ExprAST> &&expr)
        : expr(std::move(expr)) {}

    virtual llvm::Value* code_gen() override;
};

class IfStatementAST : public StatementAST {
private:
    std::unique_ptr<ExprAST> cond;
    std::unique_ptr<CodeBlockAST> if_block;
    std::unique_ptr<CodeBlockAST> else_block;
public:
    IfStatementAST(
        std::unique_ptr<ExprAST>&& cond,
        std::unique_ptr<CodeBlockAST>&& if_block,
        std::unique_ptr<CodeBlockAST>&& else_block)
        : cond(std::move(cond)),
          if_block(std::move(if_block)),
          else_block(std::move(else_block)) {}

    virtual llvm::Value* code_gen() override;
};

class ForStatementAST : public StatementAST {
private:
    std::string var_name;
    std::unique_ptr<StatementAST> start;
    std::unique_ptr<ExprAST> cond;
    std::unique_ptr<StatementAST> step;
    std::unique_ptr<CodeBlockAST> for_block;

public:
    ForStatementAST(const std::string& var_name, std::unique_ptr<StatementAST>&& start,
               std::unique_ptr<ExprAST>&& cond, std::unique_ptr<StatementAST>&& step,
               std::unique_ptr<CodeBlockAST>&& for_block)
            : var_name(var_name),
              start(std::move(start)),
              cond(std::move(cond)),
              step(std::move(step)),
              for_block(std::move(for_block)) {}

    virtual llvm::Value *code_gen() override ;
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

    inline const std::string& get_name() const {
        return name;
    }
};

class CodeBlockAST {
public:
    std::vector<std::unique_ptr<StatementAST>> statements;
    llvm::BasicBlock* code_gen();

    CodeBlockAST(std::vector<std::unique_ptr<StatementAST>>&& statements)
        : statements(std::move(statements)) {}
};

class FunctionAST {
    std::unique_ptr<PrototypeAST> proto;
    std::vector<std::unique_ptr<StatementAST>> body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST>&& proto, std::vector<std::unique_ptr<StatementAST>>&& body)
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

    std::unique_ptr<VariableExprAST> parse_variable_expr();

    std::unique_ptr<ExprAST> parse_str_expr();

    std::unique_ptr<ExprAST> parse_primary();

    std::unique_ptr<ExprAST> parse_bin_op_right(int, std::unique_ptr<ExprAST>);

    std::unique_ptr<StatementAST> parse_statement();

    std::unique_ptr<StatementAST> parse_if_statement();

    std::unique_ptr<StatementAST> parse_for_statement();

    std::unique_ptr<StatementAST> parse_for_ctrl_statement();

    std::unique_ptr<StatementAST> parse_call_statement();

    std::unique_ptr<DeclareStatementAST> parse_declare();

    std::unique_ptr<CodeBlockAST> parse_code_block();

    std::unique_ptr<PrototypeAST> parse_prototype();

    std::unique_ptr<FunctionAST> parse_definition();

    void handle_definition();

    void handle_global_declare();

    void main_loop();

    Parser(const std::vector<Token>& tokens)
        : tokens(tokens), token_it(this->tokens.begin()) {}
};


#endif //NAIVEC_PARSER_H
