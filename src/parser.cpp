//
// Created by TianYi Zhuang on 08/12/2016.
//

#include "parser.h"

std::map<char, int> Parser::precedence_map = {
    {'<', 10},
    {'+', 20},
    {'-', 20},
    {'*', 40},
};

std::unique_ptr<ExprAST> Parser::parse_number_expr() {
    std::unique_ptr<ExprAST> result = std::make_unique<NumberExprAST>(std::stoi(cur_token.value));
    cur_token = lexer.get_token();
    return result;
}

std::unique_ptr<ExprAST> Parser::parse_paren_expr() {
    cur_token = lexer.get_token(); // remove '('
    std::unique_ptr<ExprAST> v = parse_number_expr();

    if (!v) {
        return nullptr;
    }

    if (cur_token.type != T_PAREN_R) {
        // TODO
        throw std::exception();
    }

    cur_token = lexer.get_token(); // remove ')'

    return v;
}

std::unique_ptr<ExprAST> Parser::parse_identifier_expr() {
    std::string id_name = cur_token.value;
    cur_token = lexer.get_token();
    if (cur_token.type != T_PAREN_L) {
        return std::make_unique<VariableExprAST>(id_name);
    }

    cur_token = lexer.get_token();

    std::vector<std::unique_ptr<ExprAST>> args;

    while (true) {
        std::unique_ptr<ExprAST> arg = parse_expr();
        if (!arg) {
            return nullptr;
        }
        args.push_back(std::move(arg));
        if (cur_token.type == T_PAREN_R) {
            break;
        }

        if (cur_token.type != T_COMMA) {
            // TODO
            throw std::exception();
        }

        cur_token = lexer.get_token(); // remove ','
    }

    cur_token = lexer.get_token(); // remove ')'
    return std::make_unique<CallExprAST>(id_name, std::move(args));
}

std::unique_ptr<ExprAST> Parser::parse_primary() {
    switch (cur_token.type) {
        case T_IDENTIFIER:
            return parse_identifier_expr();
        case T_NUMBER:
            return parse_number_expr();
        case T_PAREN_L:
            return parse_paren_expr();
        default:
            // TODO
            throw std::exception();
    }
}

int Parser::get_token_prec() {
    if (cur_token.type != T_OP) {
        return -1;
    }

    int token_prec = precedence_map[cur_token.value[0]];

    return (token_prec <= 0) ? -1 : token_prec;
}

std::unique_ptr<ExprAST> Parser::parse_bin_op_right(int expr_prec, std::unique_ptr<ExprAST> left) {
    while (true) {
        int token_prec = get_token_prec();
        if (token_prec < expr_prec) {
            return left;
        }

        char op = cur_token.value[0];
        cur_token = lexer.get_token();
        std::unique_ptr<ExprAST> right = parse_primary();
        if (!right) {
            return nullptr;
        }

        int next_prec = precedence_map[cur_token.value[0]];

        if (token_prec < next_prec) {
            right = parse_bin_op_right(token_prec + 1, std::move(right));
        }

        left = std::make_unique<BinaryExprAST>(op, std::move(left), std::move(right));
    }
}

std::unique_ptr<ExprAST> Parser::parse_expr() {
    std::unique_ptr<ExprAST> left = parse_primary();
    if (!left) {
        return nullptr;
    }

    return parse_bin_op_right(0, std::move(left));
}

std::unique_ptr<PrototypeAST> Parser::parse_prototype() {
    if (cur_token.type != T_IDENTIFIER) {
        throw std::exception();
    }

    std::string fn_name = cur_token.value;
    cur_token = lexer.get_token();

    if (cur_token.type != T_PAREN_L) {
        throw std::exception();
    }

    std::vector<std::string> arg_names;
    while ((cur_token = lexer.get_token()).type == T_IDENTIFIER) {
        arg_names.push_back(cur_token.value);
    }

    if (cur_token.type != T_PAREN_R) {
        throw std::exception();
    }

    cur_token = lexer.get_token();

    return std::make_unique<PrototypeAST>(fn_name, std::move(arg_names));
}

std::unique_ptr<FunctionAST> Parser::parse_definition() {
    cur_token = lexer.get_token();
    std::unique_ptr<PrototypeAST> proto = parse_prototype();
    if (!proto) {
        return nullptr;
    }

    if (std::unique_ptr<ExprAST> expr = parse_expr()) {
        return std::make_unique<FunctionAST>(std::move(proto), std::move(expr));
    }

    return nullptr;
}

void Parser::main_loop() {
    while (true) {
        switch (cur_token.type) {
            case T_EOF:
                return;
            case T_SEMICOLON:
                cur_token = lexer.get_token();
                break;
            case T_DEF:
                parse_definition();
                break;
            default:
                cur_token = lexer.get_token();
                break;
        }
    }
}
