#ifndef __RUNTIME_H__
#define __RUNTIME_H__

#include "ast.h"
#include "parser.h"
#include "position.h"
#include "types.h"
#include <cstdio>
#include <functional>
#include <map>
#include <string>
#include <vector>

#define CFUNC_PROTO new ast_node(true)

typedef struct rt_value {
    std::string str;
    float num;
    ast_node* body;
    std::map<std::string, rt_value*> children;
    std::vector<rt_value*> arr;
    ast_node* proto;
    std::function<rt_value*(std::vector<rt_value*>)> cfunc;

    dtype_t type;

    rt_value(float num) : num(num), type(dtype::integer) {};
    rt_value(std::string str) : str(str), type(dtype::string) {};
    rt_value(ast_node* body, ast_node* proto) : body(body), proto(proto), num(69), str("function"), type(dtype::func) {};
    rt_value(std::map<std::string, rt_value*> children) : children(children), type(dtype::object) {};
    rt_value(std::vector<rt_value*> arr) : arr(arr), type(dtype::array) {};
    rt_value(std::function<rt_value*(std::vector<rt_value*>)> cf) : cfunc(cf), proto(CFUNC_PROTO), type(dtype::cfunction) {};
    rt_value() : type(dtype::nil) {
    };

    std::string ts() {
        switch (type) {
            case dtype::integer:
                return std::to_string(this->num);

            case dtype::string:
                return "\"" + this->str + "\"";

            case dtype::nil:
                return "nil";

            case dtype::array: {
                std::string fin = "[ ";

                for (rt_value* item : arr) {
                    fin += item->ts();
                }

                return fin + " ]";
            }

            case dtype::object: {
                std::string fin = "{\n";
                auto it = children.begin();

                while (it != children.end()) {
                    std::string key = it->first;
                    rt_value* value = it->second;

                    fin += string_format("%s: %s,\n", key.c_str(), value->ts(1).c_str());
                    it++;
                }

                return fin + "}";
            }

            case dtype::func: {
                std::string fin = "function (";

                for (ast_node* parg : proto->children) {
                    fin += string_format("%s: %s", parg->symbol.c_str(), dtype_to_str(parg->data_type).c_str());
                }

                fin += string_format(") => %s", dtype_to_str(proto->data_type).c_str());

                return fin;
            }

            case dtype::cfunction:
                return "<c function>";
        }
    }

    std::string ts(int id) {
        switch (type) {
            case dtype::integer:
                return std::to_string(this->num);

            case dtype::string:
                return "\"" + this->str + "\"";

            case dtype::nil:
                return "nil";

            case dtype::array: {
                std::string fin = "[ ";

                for (rt_value* item : arr) {
                    fin += item->ts();
                }

                return fin + " ]";
            }

            case dtype::object: {
                std::string ident = std::string(id, '\t');
                std::string fin = "{\n";
                auto it = children.begin();

                while (it != children.end()) {
                    std::string key = it->first;
                    rt_value* value = it->second;

                    fin += string_format("%s%s: %s,\n", (std::string(id + 1, '\t')).c_str(), key.c_str(), value->ts(id + 1).c_str());
                    it++;
                }

                return fin + ident + "}";
            }

            case dtype::func: {
                std::string fin = "function (";

                for (ast_node* parg : proto->children) {
                    fin += string_format("%s: %s", parg->symbol.c_str(), dtype_to_str(parg->data_type).c_str());
                }

                fin += string_format(") => %s", dtype_to_str(proto->data_type).c_str());

                return fin;
            }

            case dtype::cfunction:
                return "<c function>";
        }
    }
} rt_value_t;

#endif // __RUNTIME_H__