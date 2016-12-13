//
// Created by TianYi Zhuang on 08/12/2016.
//

#include "lexer.h"
#include <cctype>

std::ostream &operator<<(std::ostream &out, Token &t) {
    out << t.type << ' ' << t.value << std::endl;

    return out;
}


Token Lexer::get_token() {
    Token token;

    while (isspace(content[ptr]) && ptr < content.size()) {
        ++ptr;
    }

    if (ptr == content.size()) {
        token.type = T_EOF;
        return token;
    }

    std::string str;

    if (isdigit(content[ptr])) {
        token.type = T_NUMBER;
        do {
            str += content[ptr];
            ++ptr;
        } while (isdigit(content[ptr]));
        token.value = str;

    } else if (isalpha(content[ptr])) {
        do {
            str += content[ptr];
            ++ptr;
        } while (isalpha(content[ptr]) || isdigit(content[ptr]));

        token.value = str;
        if (str == "def") {
            token.type = T_DEF;
        } else if (str == "var") {
            token.type = T_VAR;
        } else {
            token.type = T_IDENTIFIER;
        }
    } else if (content[ptr] == ';') {
        token.type = T_SEMICOLON;
        ++ptr;
    } else if (content[ptr] == '=') {
        ++ptr;
        if (content[ptr] == '=') {
            ++ptr;
            token.type = T_OP;
            token.value = "==";
        } else {
            token.type = T_ASSIGN;
        }
    } else if (content[ptr] == '+' || content[ptr] == '-' || content[ptr] == '*') {
        token.type = T_OP;
        token.value.push_back(content[ptr]);
        ++ptr;
    } else if (content[ptr] == '(') {
        token.type = T_PAREN_L;
        token.value.push_back(content[ptr]);
        ++ptr;
    } else if (content[ptr] == ')') {
        token.type = T_PAREN_R;
        token.value.push_back(content[ptr]);
        ++ptr;
    } else if (content[ptr] == '{') {
        token.type = T_CURLY_L;
        token.value.push_back(content[ptr]);
        ++ptr;
    } else if (content[ptr] == '}') {
        token.type = T_CURLY_R;
        token.value.push_back(content[ptr]);
        ++ptr;
    } else if (content[ptr] == '[') {
        token.type = T_SQUARE_L;
        token.value.push_back(content[ptr]);
        ++ptr;
    } else if (content[ptr] == ']') {
        token.type = T_SQUARE_R;
        token.value.push_back(content[ptr]);
        ++ptr;
    } else if (content[ptr] == ',') {
        token.type = T_COMMA;
        token.value.push_back(content[ptr]);
        ++ptr;
    } else {
        // TODO: exception
        std::cout << (int)content[ptr];
        throw std::exception();
    }

    return token;
}

std::vector<Token> Lexer::tokenize() {
    Token token;
    std::vector<Token> tokens;
    while ((token = get_token()).type != T_EOF) {
        tokens.push_back(token);
    }

    return tokens;
}
