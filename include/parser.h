#ifndef PARSER_H_
#define PARSER_H_

#include "ast.h"
#include "lexer.h"
#include "position.h"
#include "token.h"
#include "types.h"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <error.h>
#include <string>
#include <vector>
#include <format>

template<typename ... Args>
inline std::string string_format( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

typedef struct parser {
    std::vector<token_t> tokens;
    token_t last;

    std::string source;

    parser(std::string src) : source(src) {
        tokens = lexer::tokenize(source);
        last = tokens.front();
    };

    token_t eat() {
        token_t l = tokens.front();
        tokens.erase(tokens.begin());
        return l;;
    }

    token_t expect(token_type_t type) {
        token_t next = eat();
        if (next.type != type) error(string_format("expected %s at %d:%d, got %s", token_to_str(type).c_str(), next.pos.ln, next.pos.col, token_to_str(next.type).c_str()), next.pos, source).spit();
        return next;
    }

    token_t peek() {
        token_t l = tokens.front();
        return l;
    }

    bool match(token_type_t type) {
        return peek().type == type;
    }

    ast_node* parse() {
        //return ast_node(ast_type_t::ast_noop);
        ast_node* root = parse_compound();
        root->symbol = "root";

        return root;
    }

    ast_node* parse_id() {
        token_t id = eat();
        if (match(token_type_t::equals)) {
            eat();
            ast_node* node = new ast_node(ast_type_t::ast_assign, id.pos);
            node->symbol = id.value;
            
            if (match(token_type::lparen)) {
                node->value = parse_expr();
            }

            return node;
        }

        ast_node* node = new ast_node(ast_type_t::ast_identifier, id.pos);
        node->symbol = id.value;

        token_t ig = peek();

        if (match(token_type::dot)) {
            eat();
            ast_node* member = parse_id();

            node->type = ast_type_t::ast_member;
            node->value = member;
        }

        if (match(token_type::lbrace)) {
            eat();
            ast_node* idx = parse_expr();
            expect(token_type::rbrace);

            node->type = ast_type::ast_arrindex;
            node->value = idx;
        }

        if (match(token_type::lparen)) {
            ast_node* args = parse_list();
            node->type = ast_type::ast_call;
            node->value = args;
        }

        if (match(token_type::colon)) {
            eat();
            token_t d_type = expect(token_type::identifier);
            node->data_type = str_to_dtype(d_type.value);

            if (match(token_type_t::equals)) {
                eat();
                node = new ast_node(ast_type_t::ast_assign, id.pos);
                node->symbol = id.value;
                node->value = parse_expr();
                node->data_type = str_to_dtype(d_type.value);

                return node;
            }
        }

        return node;
    }

    ast_node* parse_id_raw() {
        token_t id = eat();
        ast_node* node = new ast_node(ast_type_t::ast_identifier, id.pos);
        node->symbol = id.value;

        return node;
    }

    ast_node* parse_fn() {
        ast_node* node = parse_list();

        if (match(token_type::f_assign)) {
            eat();
            ast_node* body = parse_scope();
            node->data_type = node->data_type;
            node->value = body;
            node->type = ast_type::ast_function;
        }

        return node;
    }
    
    ast_node* parse_binary() {
        token_t num = eat();
        ast_node* node = new ast_node(ast_type::ast_num_expr, num.pos);
        node->number = std::stoi(num.value);

        if (match(token_type::binaryop)) {
            token_t op = eat();
            ast_node* left = new ast_node(ast_type::ast_num_expr, num.pos);
            left->number = std::stoi(num.value);
            print_node(left);
            ast_node* right = parse_expr();

            print_node(right);

            node->type = ast_type::ast_binop;
            node->children.push_back(left);
            node->children.push_back(right);
            node->symbol = op.value;
        }

        return node;
    }

    ast_node* parse_scope() {
        token_t start = expect(token_type::lcbrace);
        ast_node* body = new ast_node(ast_type::ast_compound, start.pos);
        ast_node* le = new ast_node(ast_type::ast_noop, start.pos);

        while (peek().type != token_type::rcbrace && le->type != ast_type::ast_return) {
            le = parse_expr();
            body->children.push_back(le);
            if (match(token_type::semi)) eat();
        }

        expect(token_type::rcbrace);

        return body;
    }

    ast_node* parse_list() {
        ast_node* list = new ast_node(ast_type_t::ast_compound, eat().pos);

        if (!match(token_type::rparen)) {
            list->children.push_back(parse_expr());
            while (match(token_type::comma)) {
                eat();
                list->children.push_back(parse_expr());
            }

            expect(token_type::rparen);
        } else eat();

        if (match(token_type::colon)) {
            eat();
            token_t d_type = expect(token_type::identifier);
            list->data_type = str_to_dtype(d_type.value);
        }

        return list;
    }

    ast_node* parse_object() {
        ast_node* obj = new ast_node(ast_type::ast_object, eat().pos);
        while (!match(token_type::rcbrace)) {
            token_t key = expect(token_type::identifier);
            expect(token_type::colon);

            ast_node* value = parse_expr();
            ast_node* entry = new ast_node(ast_type::ast_member, key.pos);
            entry->symbol = key.value;
            entry->value = value;

            obj->children.push_back(entry);

            if (!match(token_type::rcbrace)) {
                expect(token_type::comma);
            }
        }

        expect(token_type::rcbrace);

        return obj;
    }

    ast_node* parse_array() {
        token_t start = expect(token_type::lbrace);
        ast_node* list = new ast_node(ast_type_t::ast_array, start.pos);

        // return [4, 0];
        list->children.push_back(parse_expr());
        while (match(token_type::comma)) {
            eat();
            list->children.push_back(parse_expr());
        }

        expect(token_type::rbrace);

        return list;
    }

    ast_node* parse_expr() {
        token_t at = peek();
        ast_node* val = new ast_node(ast_type_t::ast_noop, at.pos);

        switch (at.type) {
            case token_type::identifier:
                val = parse_id();
                break;

            case token_type::str_literal:
                val = new ast_node(ast_type_t::ast_string_expr, at.pos);
                val->symbol = eat().value;
                break;

            case token_type::num_literal:
                val = parse_binary();
                break;

            case token_type::lparen:
                val = parse_fn();
                break;

            case token_type::lbrace:
                val = parse_array();
                break;

            case token_type::lcbrace:
                val = parse_object();
                break;

            case token_type::ret:
                val = new ast_node(ast_type::ast_return, eat().pos);
                val->value = parse_expr();
                break;

            default: error(string_format("unexpected %s at %d:%d", token_to_str(at.type).c_str(), at.pos.ln, at.pos.col), at.pos, source).spit();
        }

        return val;
    }

    ast_node* parse_compound() {
        ast_node* compound = new ast_node(ast_type_t::ast_compound, peek().pos);

        while (!match(token_type_t::eof)) {
            compound->children.push_back(parse_expr());
            if (match(token_type::semi)) eat();
        }

        return compound;
    }
} parser_t;

#endif // PARSER_H_