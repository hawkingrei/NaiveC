//
// Created by TianYi Zhuang on 08/12/2016.
//

#ifndef NAIVEC_LEXER_H
#define NAIVEC_LEXER_H

#include <string>
#include <iostream>
#include <vector>

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
    T_RETURN,
    T_STAR,
    T_IF,
    T_ELSE,
    T_STR,
    T_FOR,
    T_BREAK,
    T_CHAR,
};

struct Token {
    TokenType type;
    std::string value = "";
};

std::ostream& operator<<(std::ostream& out, Token& t);

class Lexer {
private:
    std::string content;
    size_t ptr = 0;

    Token get_token();
    char get_char();

public:
    Lexer(std::string&& content) : content(content) {}

    std::vector<Token> tokenize();
};


#endif //NAIVEC_LEXER_H
