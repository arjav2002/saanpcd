#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

enum TokenType
{
    KEYWORD,
    SYMBOL,
    UN_OP,
    REL_OP,
    ASSIGN_OP,
    IDENTIFIER,
    INT_LIT,
    CHAR_LIT,
    STR_LIT,
    FLOAT_LIT,
    BOOL_LIT,
    DEFAULT,
    DTYPE,
    PROC,
    VOID,
    IF,
    ELIF,
    ELSE,
    LOOP,
    BREAK,
    CONTINUE,
    RETURN,
    AND,
    IS,
    NOR,
    XOR,
    NAND,
    OR,
    TRUE,
    FALSE,
    HASH,
    TILDE,
    ASTERIX,
    SLASH,
    MODULO,
    COMMA,
    SEMICOLON,
    EXCLAMATION,
    AMPERSAND,
    PIPE,
    KARAT,
    EQUAL,
    LESSTHAN,
    GREATERTHEN,
    BACKSLASH,
    RIGHTANGULAR,
    LEFTANGULAR,
    RIGHTSQUARE,
    LEFTSQUARE,
    LEFTROUND,
    RIGHTROUND,
    UNDERSCORE,
    DOT,
    DOUBLEQUOTE,
    SINGLEQUOTE,
    INCREMENT,
    DECREMENT,
    DOUBLEEQUAL,
    $
};

struct Token
{
    TokenType token_type;
    std::string token_name;
    int token_linenumber, dims;
    Token(TokenType tn = DEFAULT, std::string tname = "", int ln = -1, int d = -1)
    {
        token_type = tn;
        token_name = tname;
        token_linenumber = ln;
        dims = d;
    }
};

std::string tokenToString(TokenType tt);

std::string tokenToTerminal(Token t);
#endif