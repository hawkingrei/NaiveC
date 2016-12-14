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
    std::unique_ptr<ExprAST> result = std::make_unique<NumberExprAST>(std::stoi(token_it->value));
    ++token_it;
    return result;
}

std::unique_ptr<ExprAST> Parser::parse_paren_expr() {
    ++token_it; // remove '('
    std::unique_ptr<ExprAST> v = parse_expr();

    if (!v) {
        return nullptr;
    }

    if (token_it->type != T_PAREN_R) {
        // TODO
        throw std::exception();
    }

    ++token_it; // remove ')'

    return v;
}

std::unique_ptr<ExprAST> Parser::parse_identifier_expr() {
    std::string id_name = token_it->value;
    ++token_it;
    if (token_it->type != T_PAREN_L) {
        return std::make_unique<VariableExprAST>(id_name);
    }

    ++token_it;

    std::vector<std::unique_ptr<ExprAST>> args;

    if (token_it->type == T_PAREN_R) {
        ++token_it;
        return std::make_unique<CallExprAST>(id_name, std::move(args));
    }

    while (true) {
        std::unique_ptr<ExprAST> arg = parse_expr();
        if (!arg) {
            return nullptr;
        }
        args.push_back(std::move(arg));
        if (token_it->type == T_PAREN_R) {
            break;
        }

        if (token_it->type != T_COMMA) {
            // TODO
            throw std::exception();
        }

        ++token_it; // remove ','
    }

    ++token_it; // remove ')'
    return std::make_unique<CallExprAST>(id_name, std::move(args));
}

std::unique_ptr<ExprAST> Parser::parse_primary() {
    switch (token_it->type) {
        case T_IDENTIFIER:
            return parse_identifier_expr();
        case T_NUMBER:
            return parse_number_expr();
        case T_PAREN_L:
            return parse_paren_expr();
        default:
            // TODO
            std::cerr << *token_it << std::endl;
            throw std::exception();
    }
}

std::unique_ptr<DeclareAST> Parser::parse_declare() {
    std::string type = token_it->value;
    ++token_it; // remove type

    if (token_it->type != T_IDENTIFIER) {
        return nullptr;
    }

    std::string id_name = token_it->value;

    ++token_it; // remove identifier

    if (token_it->type == T_SEMICOLON) {
        ++token_it;
        return std::make_unique<DeclareAST>(type, id_name);
    }

    if (token_it->type != T_SQUARE_L) {
        throw std::exception();
    }

    ++token_it; // remove '['

    if (token_it->type != T_NUMBER) {
        throw std::exception();
    }

    int array_length = std::stoi(token_it->value);
    ++token_it;

    if (token_it->type != T_SQUARE_R) {
        throw std::exception();
    }

    ++token_it; // remove ']'

    if (token_it->type != T_SEMICOLON) {
        throw std::exception();
    }

    auto ret = std::make_unique<DeclareAST>(type, id_name, true, array_length);
    ++token_it;

    return std::move(ret);
}

int Parser::get_token_prec() {
    if (token_it->type != T_OP) {
        return -1;
    }

    int token_prec = precedence_map[token_it->value[0]];

    return (token_prec <= 0) ? -1 : token_prec;
}

std::unique_ptr<ExprAST> Parser::parse_bin_op_right(int expr_prec, std::unique_ptr<ExprAST> left) {
    while (true) {
        int token_prec = get_token_prec();
        if (token_prec < expr_prec) {
            return left;
        }

        char op = token_it->value[0];
        ++token_it;
        std::unique_ptr<ExprAST> right = parse_primary();
        if (!right) {
            return nullptr;
        }

        int next_prec = precedence_map[token_it->value[0]];

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
    if (token_it->type != T_IDENTIFIER) {
        throw std::exception();
    }

    std::string fn_name = token_it->value;
    ++token_it;

    if (token_it->type != T_PAREN_L) {
        throw std::exception();
    }

    std::vector<std::string> arg_names;
    while ((++token_it)->type == T_IDENTIFIER) {
        arg_names.push_back(token_it->value);
    }

    if (token_it->type != T_PAREN_R) {
        throw std::exception();
    }

    ++token_it;

    return std::make_unique<PrototypeAST>(fn_name, std::move(arg_names));
}

std::unique_ptr<FunctionAST> Parser::parse_definition() {
    ++token_it;
    std::unique_ptr<PrototypeAST> proto = parse_prototype();
    if (!proto) {
        return nullptr;
    }

    if (std::unique_ptr<ExprAST> expr = parse_expr()) {
        return std::make_unique<FunctionAST>(std::move(proto), std::move(expr));
    }

    return nullptr;
}

std::unique_ptr<FunctionAST> Parser::parse_top_level_expr() {
    if (auto expr = parse_expr()) {
        auto proto = std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());
        return std::make_unique<FunctionAST>(std::move(proto), std::move(expr));
    }

    return nullptr;
}

void Parser::handle_definition() {
    if (auto fn_ast = parse_definition()) {
        if (auto fn_ir = fn_ast->code_gen()) {
            fn_ir->dump();
        }
    } else {
        ++token_it;
    }
}

void Parser::handle_top_level_expr() {
    if (auto fn_ast = parse_top_level_expr()) {
        if (auto fn_ir = fn_ast->code_gen()) {
            fn_ir->dump();
        }
    } else {
        ++token_it;
    }
}

void Parser::handle_global_declare() {
    if (auto d = parse_declare()) {
        d->set_global(true);
        if (auto d_ir = d->code_gen()) {
            d_ir->dump();
        }
    } else {
        ++token_it;
    }
}

void Parser::main_loop() {
    while (token_it != tokens.end()) {
        switch (token_it->type) {
            case T_EOF:
                return;
            case T_SEMICOLON:
                ++token_it;
                break;
            case T_DEF:
                handle_definition();
                break;
            case T_TYPE:
                handle_global_declare();
                break;
            default:
                handle_top_level_expr();
                break;
        }
    }
}
