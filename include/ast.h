#ifndef AST_H_
#define AST_H_

#include "position.h"
#include "token.h"
#include "types.h"
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include "types.h"

typedef enum struct ast_type {
        ast_compound,
        ast_noop,

        ast_assign,

        ast_string_expr,
        ast_num_expr,
        ast_identifier,
        ast_return,
        ast_call,
        ast_binop,
        ast_array,
        ast_object,
        ast_member,
        ast_arrindex,
        ast_function,
        ast_import,
        ast_if,
        ast_while,
        ast_bool
} ast_type_t;

struct ast_node {
    std::vector<ast_node*> children;
    ast_type_t type;
    position_t pos;
    ast_node* value;
    std::string symbol;
    float number;
    dtype_t data_type;
    ast_node* svalue;

    ast_node(ast_type_t type, position_t p) : type(type), pos(p) {};
    ast_node() : type(
        ast_type::ast_noop
    ), pos() {};

    ast_node(bool a) : type(
        ast_type::ast_compound
    ), pos() {
        this->children.push_back(new ast_node());
        this->children.push_back(new ast_node());
        this->children.push_back(new ast_node());
        this->children.push_back(new ast_node());
        this->children.push_back(new ast_node());
        this->children.push_back(new ast_node());
        this->children.push_back(new ast_node());
        this->data_type = dtype::cfunction;
    };
};

inline std::string ast_to_string(ast_type_t type) {
    switch (type) {
        case ast_type::ast_return:
            return "return statement";

        case ast_type::ast_assign:
            return "assign statement";

        case ast_type::ast_compound:
            return "compound";

        case ast_type::ast_function:
            return "function";

        case ast_type::ast_identifier:
            return "identifier";

        case ast_type::ast_noop:
            return "no operation";

        case ast_type::ast_num_expr:
            return "number expression";

        case ast_type::ast_string_expr:
            return "string expression";

        case ast_type::ast_call:
            return "call expression";

        case ast_type::ast_binop:
            return "binary operation";

        case ast_type::ast_array:
            return "array expression";

        case ast_type::ast_object:
            return "object expression";

        case ast_type::ast_arrindex:
            return "array indexation";

        case ast_type::ast_member:
            return "member expression";

        case ast_type::ast_import:
            return "import statement";

        case ast_type::ast_bool:
            return "boolean expression";

        case ast_type::ast_if:
            return "if statement";
        
        case ast_type::ast_while:
            return "while statement";
    }
}

inline void print_node(ast_node* node, int tl) {
    std::string idents = std::string(tl, ' ');
    printf(
        "%s<type:%s, pos:%d:%d, symbol:%s, number: %f, dtype:%s>\n",
        idents.c_str(), ast_to_string(node->type).c_str(),
        node->pos.ln, node->pos.col, node->symbol.c_str(),
        node->number, dtype_to_str(node->data_type).c_str()
    );

    if (node->children.size() >= 1) {
        printf("%sCHILDREN:\n", idents.c_str());
        for (ast_node* elem : node->children) {
            //printf("    <type:%d, pos:%d:%d, symbol:%s, number: %f>\n", static_cast<int>(elem->type), elem->pos.ln, elem->pos.col, elem->symbol.c_str(), elem->number);
            print_node(elem, tl + 1);
        }
    }

    if (node->value != nullptr) {
        printf("%sVALUE:\n", idents.c_str());
        print_node(node->value, tl + 1);
    }

    if (node->svalue != nullptr) {
        printf("%sSVALUE:\n", idents.c_str());
        print_node(node->value, tl + 1);
    }
}

inline void print_node(ast_node* node) {
    std::string idents = std::string(0, ' ');
    printf(
        "%s<type:%s, pos:%d:%d, symbol:%s, number: %f, dtype:%s>\n",
        idents.c_str(), ast_to_string(node->type).c_str(),
        node->pos.ln, node->pos.col, node->symbol.c_str(),
        node->number, dtype_to_str(node->data_type).c_str()
    );

    if (node->children.size() >= 1) {
        printf("%sCHILDREN:\n", idents.c_str());
        for (ast_node* elem : node->children) {
            //printf("    <type:%d, pos:%d:%d, symbol:%s, number: %f>\n", static_cast<int>(elem->type), elem->pos.ln, elem->pos.col, elem->symbol.c_str(), elem->number);
            print_node(elem, 1);
        }
    }

    if (node->value != nullptr) {
        printf("%sVALUE:\n", idents.c_str());
        print_node(node->value, 1);
    }

    if (node->svalue != nullptr) {
        printf("%sSVALUE:\n", idents.c_str());
        print_node(node->value, 1);
    }
}

#endif // AST_H_