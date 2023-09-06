#ifndef LEXER_H_
#define LEXER_H_

#include "position.h"
#include "token.h"
#include <cctype>
#include <cstdio>
#include <error.h>
#include <string>
#include <map>
#include <vector>

inline std::map<std::string, token_type_t> KEYWORDS;

namespace lexer {
    inline void init_kw_def() {
        KEYWORDS["return"] = token_type::ret;
        KEYWORDS["import"] = token_type::import;
        KEYWORDS["as"] = token_type::as;
        KEYWORDS["if"] = token_type::if_t;
        KEYWORDS["while"] = token_type::while_t;
        KEYWORDS["else"] = token_type::else_t;

        KEYWORDS["true"] = token_type::true_t;
        KEYWORDS["false"] = token_type::false_t;
    }
    
    inline std::vector<token_t> tokenize(const std::string& str) {
        init_kw_def();

        std::vector<token_t> tokens;
        std::string buf;
        position_t pos;
        
        for (int i = 0; i < str.length(); i++) {
            pos.col++;
            char c = str.at(i);

            if (c == '\n') {pos.ln++; pos.col = 1;}
            if (std::isspace(c)) {
                continue;
            }

            if (std::isalpha(c)) {
                buf.push_back(c); i++; pos.col++;
                while (std::isalnum(str.at(i))) {
                    c = str.at(i);
                    if (c == '\n') {pos.ln++; pos.col = 1;}
                    buf.push_back(c); i++; pos.col++;
                }; i--; pos.col--;

                if (KEYWORDS.find(buf) != KEYWORDS.end()) {
                    tokens.push_back(token(KEYWORDS[buf], buf, pos));
                    buf.clear(); continue;
                } else {
                    tokens.push_back(token(token_type::identifier, buf, pos));
                    buf.clear(); continue;
                }
            }

            if (std::isdigit(c)) {
                buf.push_back(c); i++; pos.col++;
                while (std::isdigit(str.at(i))) {
                    c = str.at(i);
                    if (c == '\n') {pos.ln++; pos.col = 1;}
                    buf.push_back(c); i++; pos.col++;
                }; i--; pos.col--;

                tokens.push_back(token(token_type::num_literal, buf, pos));
                buf.clear(); continue;
            }

            if (c == '"') {
                pos.col++; i++;
                while (str.at(i) != '"') {
                    c = str.at(i);
                    if (c == '\n') {pos.ln++; pos.col = 1;}
                    buf.push_back(c); i++; pos.col++;
                };

                tokens.push_back(token(token_type::str_literal, buf, pos));
                buf.clear(); continue;
            }

            if (c == ';') {
                tokens.push_back(token(token_type::semi, ";", pos)); continue;
            }

            if (c == '=') {
                if (str.at(i + 1) == '>') {
                    i++; pos.col++;
                    tokens.push_back(token(token_type::f_assign, "=>", pos)); continue;
                }

                if (str.at(i + 1) == '=') {
                    i++; pos.col++;
                    tokens.push_back(token(token_type::binaryop, "==", pos)); continue;
                }

                tokens.push_back(token(token_type::equals, "=", pos)); continue;
            }

            if (c == '+' || c == '-' || c == '*' || c == '/') {
                tokens.push_back(token(token_type::binaryop, std::string(1, c), pos)); continue;
            }

            if (c == '>') {
                if (str.at(i + 1) == '=') {
                    i++; pos.col++;
                    tokens.push_back(token(token_type::binaryop, ">=", pos)); continue;
                }

                tokens.push_back(token(token_type::binaryop, std::string(1, c), pos)); continue;
            }

            if (c == '<') {
                if (str.at(i + 1) == '=') {
                    i++; pos.col++;
                    tokens.push_back(token(token_type::binaryop, "<=", pos)); continue;
                }

                tokens.push_back(token(token_type::binaryop, std::string(1, c), pos)); continue;
            }

            if (c == '(') {
                tokens.push_back(token(token_type::lparen, "(", pos)); continue;
            }

            if (c == ')') {
                tokens.push_back(token(token_type::rparen, ")", pos)); continue;
            }

            if (c == '{') {
                tokens.push_back(token(token_type::lcbrace, "{", pos)); continue;
            }

            if (c == '}') {
                tokens.push_back(token(token_type::rcbrace, "}", pos)); continue;
            }

            if (c == '[') {
                tokens.push_back(token(token_type::lbrace, "[", pos)); continue;
            }

            if (c == ']') {
                tokens.push_back(token(token_type::rbrace, "]", pos)); continue;
            }

            if (c == ',') {
                tokens.push_back(token(token_type::comma, ",", pos)); continue;
            }

            if (c == ':') {
                tokens.push_back(token(token_type::colon, ":", pos)); continue;
            }

            if (c == '.') {
                tokens.push_back(token(token_type::dot, ":", pos)); continue;
            }

            error("unexpected character", pos, str).spit();
        }

        tokens.push_back(token(token_type::eof, "\0", pos));

        return tokens;
    }
}


#endif // LEXER_H_