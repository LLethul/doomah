#ifndef TOKEN_H_
#define TOKEN_H_

#include "position.h"
#include <string>
typedef enum struct token_type {
    ret,
    identifier, str_literal, num_literal,
    binaryop, equals, 
    semi, eof, colon, lparen, rparen, lcbrace, rcbrace, lbrace, rbrace, comma, f_assign, dot,
    import,
} token_type_t;

typedef struct token {
    token_type_t type;
    std::string value;
    position_t pos;

    token(token_type_t t, std::string v, position_t pos) 
    : type(t), value(v), pos(pos) {};

    token() {};
} token_t;

inline std::string token_to_str(token_type_t type) {
    switch (type) {
        case token_type::str_literal:
            return "string";

        case token_type::num_literal:
            return "number";

        case token_type::binaryop:
            return "binary operator";
        
        case token_type::ret:
            return "return statement";

        case token_type::colon:
            return "colon";

        case token_type::identifier:
            return "identifier";

        case token_type::eof:
            return "<eof>";

        case token_type::semi:
            return "semicolon";

        case token_type::equals:
            return "equals";

        case token_type::lparen:
            return "left parenthesis";

        case token_type::rparen:
            return "right parenthesis";

        case token_type::lcbrace:
            return "left curly bracket";

        case token_type::rcbrace:
            return "right curly bracket";

        case token_type::lbrace:
            return "left bracket";

        case token_type::rbrace:
            return "right bracket";

        case token_type::comma:
            return "comma";

        case token_type::f_assign:
            return "function assign start";

        case token_type::dot:
            return "dot";

        case token_type::import:
            return "import statement";
    }
}

inline void print_tok(token_t tok) {
    printf("<type:%d, pos:%d:%d, value:%s>\n", static_cast<int>(tok.type), tok.pos.ln, tok.pos.col, tok.value.c_str());
}

#endif // TOKEN_H_