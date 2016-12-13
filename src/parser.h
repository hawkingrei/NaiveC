//
// Created by TianYi Zhuang on 08/12/2016.
//

#ifndef NAIVEC_PARSER_H
#define NAIVEC_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "lexer.h"

class ExprAST {
public:
    virtual ~ExprAST() {}
};

class NumberExprAST : public ExprAST {
private:
    int value;
public:
    NumberExprAST(int value) : value(value) {}
};

class VariableExprAST : public ExprAST {
private:
    std::string name;
public:
    VariableExprAST(std::string name) : name(name) {}
};

class BinaryExprAST : public ExprAST {
private:
    char op;
    std::unique_ptr<ExprAST> left;
    std::unique_ptr<ExprAST> right;
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST>&& left, std::unique_ptr<ExprAST>&& right)
        : op(op), left(std::move(left)), right(std::move(right)) {}
};

class CallExprAST : public ExprAST {
private:
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;
public:
    CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>>&& args)
        : callee(callee), args(std::move(args)) {}
};

class PrototypeAST {
    std::string name;
    std::vector<std::string> args;

public:
    PrototypeAST(const std::string& name, std::vector<std::string> args)
        : name(name), args(args) {}
};

class FunctionAST {
    std::unique_ptr<PrototypeAST> proto;
    std::unique_ptr<ExprAST> body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST>&& proto, std::unique_ptr<ExprAST>&& body)
        : proto(std::move(proto)), body(std::move(body)) {}
};


class Parser {
private:
    Lexer lexer;
    Token cur_token;
    static std::map<char, int> precedence_map;

    int get_token_prec();

public:

    std::unique_ptr<ExprAST> parse_expr();

    std::unique_ptr<ExprAST> parse_number_expr();

    std::unique_ptr<ExprAST> parse_paren_expr();

    std::unique_ptr<ExprAST> parse_identifier_expr();

    std::unique_ptr<ExprAST> parse_primary();

    std::unique_ptr<ExprAST> parse_bin_op_right(int, std::unique_ptr<ExprAST>);

    std::unique_ptr<PrototypeAST> parse_prototype();

    std::unique_ptr<FunctionAST> parse_definition();

    void main_loop();

    Parser(const Lexer& lexer) : lexer(lexer) {}
};


#endif //NAIVEC_PARSER_H
