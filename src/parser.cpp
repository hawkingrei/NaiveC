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

inline void Parser::assert_token(TokenType token_type) throw() {
    if (token_it->type == token_type) {
        return;
    }

    // TODO:
    std::cerr << "Should be token " << token_type << std::endl;
    std::cerr << "Here is " << *token_it << std::endl;
    std::cerr << std::endl;
    throw std::exception();
}

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

    assert_token(T_PAREN_R);

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

        assert_token(T_COMMA);

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

std::unique_ptr<DeclareStatementAST> Parser::parse_declare() {
    std::string type = token_it->value;
    ++token_it; // remove type

    if (token_it->type != T_IDENTIFIER) {
        return nullptr;
    }

    std::string id_name = token_it->value;

    ++token_it; // remove identifier

    if (token_it->type == T_SEMICOLON) {
        ++token_it;
        return std::make_unique<DeclareStatementAST>(type, id_name);
    }

    assert_token(T_SQUARE_L);
    ++token_it; // remove '['

    assert_token(T_NUMBER);
    int array_length = std::stoi(token_it->value);
    ++token_it;

    assert_token(T_SQUARE_R);
    ++token_it; // remove ']'

    assert_token(T_SEMICOLON);
    ++token_it;

    return std::make_unique<DeclareStatementAST>(type, id_name, true, array_length);
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

std::unique_ptr<StatementAST> Parser::parse_statement() {
    if (token_it->type == T_TYPE) {
        return parse_declare();
    }

    if (token_it->type == T_RETURN) {
        ++token_it;
        std::unique_ptr<ExprAST> expr = parse_expr();

        assert_token(T_SEMICOLON);
        ++token_it;
        return std::make_unique<ReturnStatementAST>(std::move(expr));
    }

    std::unique_ptr<ExprAST> expr1 = parse_expr();

    if (token_it->type != T_ASSIGN) {
        return nullptr;
    }

    ++token_it; // remove '='

    std::unique_ptr<ExprAST> expr2 = parse_expr();

    assert_token(T_SEMICOLON);
    ++token_it;

    return std::make_unique<AssignStatementAST>(std::move(expr1), std::move(expr2));
}

std::unique_ptr<PrototypeAST> Parser::parse_prototype() {
    assert_token(T_TYPE);
    std::string ret_type = token_it->value;
    ++token_it;

    assert_token(T_IDENTIFIER);
    std::string fn_name = token_it->value;
    ++token_it;

    assert_token(T_PAREN_L);
    std::vector<std::string> arg_names;
    while ((++token_it)->type == T_IDENTIFIER) {
        arg_names.push_back(token_it->value);
    }

    assert_token(T_PAREN_R);
    ++token_it;

    return std::make_unique<PrototypeAST>(ret_type, fn_name, std::move(arg_names));
}

std::unique_ptr<FunctionAST> Parser::parse_definition() {
    std::unique_ptr<PrototypeAST> proto = parse_prototype();
    if (!proto) {
        return nullptr;
    }

    assert_token(T_CURLY_L);
    ++token_it; // remove '{'

    std::vector<std::unique_ptr<StatementAST>> body;

    while (token_it->type != T_CURLY_R) {
        auto statement = parse_statement();
        if (statement) {
            body.push_back(std::move(statement));
        }
    }

    ++token_it; // remove '}'

    return std::make_unique<FunctionAST>(std::move(proto), std::move(body));
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
    // lookahead 3 characters
    while (tokens.end() - token_it > 2) {
        assert_token(T_TYPE);
        switch ((token_it + 2)->type) {
            case T_PAREN_L:
                handle_definition();
                break;
            default:
                handle_global_declare();
                break;
        }
    }
}
