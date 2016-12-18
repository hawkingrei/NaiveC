//
// Created by TianYi Zhuang on 08/12/2016.
//

#include "parser.h"
#include <string>

using namespace parser;

std::map<std::string, int> Parser::precedence_map = {
    {"<", 10},
    {"==", 10},
    {"!=", 10},
    {"+", 20},
    {"-", 20},
    {"*", 40},
};

inline void Parser::assert_token(TokenType token_type) throw() {
    if (token_it->type == token_type) {
        return;
    }

    // TODO:
    std::cerr << "Should be token " << token_type << std::endl;
    std::cerr << "Here is " << token_it->type << " " << token_it->value << std::endl;
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
    if ((token_it + 1)->type != T_PAREN_L) {
        return parse_variable_expr();
    }
    std::string id_name = token_it->value;
    ++token_it; // consume id
    ++token_it; // consume (

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

std::unique_ptr<ExprAST> Parser::parse_str_expr() {
    const std::string content = token_it->value;
    ++token_it;
    return std::make_unique<StrAST>(content);
}

std::unique_ptr<ExprAST> Parser::parse_primary() {
    switch (token_it->type) {
        case T_IDENTIFIER:
            return parse_identifier_expr();
        case T_NUMBER:
            return parse_number_expr();
        case T_PAREN_L:
            return parse_paren_expr();
        case T_STR:
            return parse_str_expr();
        default:
            // TODO
            std::cerr << *token_it << std::endl;
            throw std::exception();
    }
}

std::unique_ptr<DeclareStatementAST> Parser::parse_declare() {
    std::string type_name = token_it->value;
    ++token_it; // remove type

    if (token_it->type != T_IDENTIFIER) {
        return nullptr;
    }

    std::string id_name = token_it->value;

    ++token_it; // remove identifier

    if (token_it->type == T_SEMICOLON) {
        ++token_it;
        auto type_p = std::make_unique<VarType>(type_name);
        return std::make_unique<DeclareStatementAST>(std::move(type_p), id_name);
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

    auto raw_type_p = std::make_unique<VarType>(type_name);

    return std::make_unique<DeclareStatementAST>(std::make_unique<VarType>(std::move(raw_type_p), array_length), id_name);
}

std::unique_ptr<CodeBlockAST> Parser::parse_code_block() {
    std::vector<std::unique_ptr<StatementAST>> statements;
    assert_token(T_CURLY_L);
    ++token_it;

    while (token_it->type != T_CURLY_R) {
        statements.push_back(parse_statement());
    }

    ++token_it;

    return std::make_unique<CodeBlockAST>(std::move(statements));
}

int Parser::get_token_prec() {
    if (token_it->type != T_OP) {
        return -1;
    }

    int token_prec = precedence_map[token_it->value];

    return (token_prec <= 0) ? -1 : token_prec;
}

std::unique_ptr<ExprAST> Parser::parse_bin_op_right(int expr_prec, std::unique_ptr<ExprAST> left) {
    while (true) {
        int token_prec = get_token_prec();
        if (token_prec < expr_prec) {
            return left;
        }

        std::string op = token_it->value;
        ++token_it;
        std::unique_ptr<ExprAST> right = parse_primary();
        if (!right) {
            return nullptr;
        }

        int next_prec = precedence_map[token_it->value];

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

    if (token_it->type == T_IF) {
        return parse_if_statement();
    }

    if (token_it->type == T_FOR) {
        return parse_for_statement();
    }

    if ((token_it + 1)->type == T_PAREN_L) {
        return parse_call_statement();
    }

    std::unique_ptr<VariableExprAST> expr_l = parse_variable_expr();

    if (token_it->type != T_ASSIGN) {
        return nullptr;
    }

    ++token_it; // remove '='

    std::unique_ptr<ExprAST> expr_r = parse_expr();

    assert_token(T_SEMICOLON);
    ++token_it;

    return std::make_unique<AssignStatementAST>(std::move(expr_l), std::move(expr_r));
}

std::unique_ptr<StatementAST> Parser::parse_if_statement() {
    assert_token(T_IF);
    ++token_it;

    assert_token(T_PAREN_L);
    ++token_it;

    std::unique_ptr<ExprAST> cond = parse_expr();

    assert_token(T_PAREN_R);
    ++token_it;

    std::unique_ptr<CodeBlockAST> if_block = parse_code_block();

    if (token_it->type != T_ELSE) {
        return std::make_unique<IfStatementAST>(std::move(cond), std::move(if_block), nullptr);
    }
    ++token_it;

    std::unique_ptr<CodeBlockAST> else_block = parse_code_block();

    return std::make_unique<IfStatementAST>(std::move(cond), std::move(if_block), std::move(else_block));
}

std::unique_ptr<StatementAST> Parser::parse_for_ctrl_statement() {
    assert_token(T_IDENTIFIER);
    std::unique_ptr<VariableExprAST> expr_l = parse_variable_expr();

    assert_token(T_ASSIGN);
    ++token_it; // remove '='

    std::unique_ptr<ExprAST> expr_r = parse_expr();

    return std::make_unique<AssignStatementAST>(std::move(expr_l), std::move(expr_r));
}

std::unique_ptr<StatementAST> Parser::parse_call_statement() {
    std::unique_ptr<ExprAST> expr = parse_identifier_expr();
    assert_token(T_SEMICOLON);
    ++token_it;
    return std::make_unique<CallStatement>(std::move(expr));
}

std::unique_ptr<StatementAST> Parser::parse_for_statement() {
    assert_token(T_FOR);
    ++token_it; //consume for

    assert_token(T_PAREN_L);
    ++token_it; //consume (

    assert_token(T_IDENTIFIER);
    std::string var_name = token_it->value;

    std::unique_ptr<StatementAST> start = parse_for_ctrl_statement();
    assert_token(T_SEMICOLON);
    ++token_it; //consume ;

    std::unique_ptr<ExprAST> cond = parse_expr();
    assert_token(T_SEMICOLON);
    ++token_it; //consume ;

    std::unique_ptr<StatementAST> step = parse_for_ctrl_statement();

    assert_token(T_PAREN_R);
    ++token_it; //consume (

    std::unique_ptr<CodeBlockAST> for_block = parse_code_block();
    return std::make_unique<ForStatementAST>(var_name, std::move(start),
                                             std::move(cond), std::move(step), std::move(for_block));
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

std::unique_ptr<VariableExprAST> Parser::parse_variable_expr() {
    assert_token(T_IDENTIFIER);
    std::string id_name = token_it->value;
    ++token_it;

    if (token_it->type == T_SQUARE_L) {
        ++token_it;

        std::unique_ptr<ExprAST> index = nullptr;
        switch (token_it->type) {
            case T_NUMBER:
                index = parse_number_expr();
                break;
            case T_IDENTIFIER:
                index = parse_variable_expr();
                break;
            default:
                // TODO
                std::cerr << "Index not support" << std::endl;
                throw std::exception();
        }

        assert_token(T_SQUARE_R);
        ++token_it;
        return std::make_unique<VariableExprAST>(id_name, true, std::move(index));
    } else {
        return std::make_unique<VariableExprAST>(id_name);
    }
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
