//
// Created by TianYi Zhuang on 08/12/2016.
//

#ifndef NAIVEC_LEXER_H
#define NAIVEC_LEXER_H

#include <string>
#include <iostream>

enum TokenType {
    T_NUMBER = 0,
    T_IDENTIFIER,
    T_EOF,
    T_TYPE,
    T_SEMICOLON,
    T_ASSIGN,
    T_OP,
    T_PAREN_L,
    T_PAREN_R,
    T_CURLY_L,
    T_CURLY_R,
    T_SQUARE_L,
    T_SQUARE_R,
    T_COMMA,
    T_DEF,
};

struct Token {
    TokenType type;
    std::string value = "";
};

std::ostream &operator<<(std::ostream &out, Token &t);

class Lexer {
private:
    std::string content;
    size_t ptr = 0;

public:
    Lexer(std::string &&content) : content(content) {}
    Token get_token();
};


#endif //NAIVEC_LEXER_H
