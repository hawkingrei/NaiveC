//
// Created by TianYi Zhuang on 08/12/2016.
//

#include "lexer.h"
#include <cctype>

std::ostream &operator<<(std::ostream &out, Token &t) {
    out << t.type << ' ' << t.value << std::endl;

    return out;
}

char Lexer::get_char() {
    char c;
    if (content[ptr] == '\\') {
        ++ptr;
        if (content[ptr] == 'n') {
            c = '\n';
            ++ptr;
        } else {
            c = '\\';
        }
    } else {
        c = content[ptr];
        ++ptr;
    }
    return c;
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
        } else if (str == "int" || str == "char") {
            token.type = T_TYPE;
        } else if (str == "return") {
            token.type = T_RETURN;
        } else if (str == "if") {
            token.type = T_IF;
        } else if (str == "else") {
            token.type = T_ELSE;
        } else if (str == "for") {
            token.type = T_FOR;
        } else if (str == "break") {
            token.type = T_BREAK;
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
    } else if (content[ptr] == '+' || content[ptr] == '-' || content[ptr] == '*' || content[ptr] == '<') {
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
    } else if (content[ptr] == '"') {
        do {
            str += get_char();
        } while (content[ptr] != '"');

        ++ptr;
        token.type = T_STR;
        token.value = str.substr(1);
    } else if (content[ptr] == '\''){
        token.type = T_CHAR;
        token.value.push_back(get_char());
        if (content[ptr] != '\'') {
            std::cout << (char)content[ptr];
            throw std::exception();
        }
        ++ptr;
    } else {
        // TODO: exception
        std::cout << (char)content[ptr];
        throw std::exception();
    }

    return token;
}

std::vector<Token> Lexer::tokenize() {
    Token token;
    std::vector<Token> tokens;
    while ((token = get_token()).type != T_EOF) {
        if (tokens.back().type == T_TYPE && token.type == T_OP && token.value == "*") {
            token.type = T_STAR;
        }

        tokens.push_back(token);
    }

    return tokens;
}
