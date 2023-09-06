#include "interpreter.h"
#include "ast.h"
#include "builtin.h"
#include "env.h"
#include "futil.h"
#include "parser.h"
#include "position.h"
#include "runtime.h"
#include "types.h"
#include <cstdio>
#include <error.h>
#include <string>
#include <vector>
#include <iostream>

rt_value* print(std::vector<rt_value*> args, void* env) {
    //std::string fin;
    for (int i = 0; i < args.size(); i++) {
        auto elem = args[i];
        if (i != args.size() - 1) {
            elem->out(); printf(", ");
        } else {
            elem->out(); printf("\n");
        }
    }

    //printf("%s\n", fin.c_str());
    return new rt_value();
}

std::string repeat(std::string str, const std::size_t n)
{
    if (n == 0) {
        str.clear();
        str.shrink_to_fit();
        return str;
    } else if (n == 1 || str.empty()) {
        return str;
    }
    const auto period = str.size();
    if (period == 1) {
        str.append(n - 1, str.front());
        return str;
    }
    str.reserve(period * n);
    std::size_t m {2};
    for (; m < n; m *= 2) str += str;
    str.append(str.c_str(), (n - (m / 2)) * period);
    return str;
}

#include <functional>
#include <type_traits>

rt_value_t* interpreter::run()
{
    rt_value_t* rt_val;
    environment_t* scope = new environment();
    def_on_env(scope);
    scope->interpret = this;

    // rlly hacky but
    rt_value* arg0;

    scope->assign("print", new rt_value((std::function<rt_value*(std::vector<rt_value*>, void*)>)print));

    ast_node* root = p.parse();
    //print_node(root);
    rt_val = eval_scope_samenv(root, scope);

    return rt_val;
}

rt_value_t* interpreter::eval(ast_node* node, environment_t* env)
{
    if (node == nullptr) return new rt_value();
    switch (node->type) {
        case ast_type::ast_identifier:
            return env->get_var(node->symbol);

        case ast_type::ast_return:
            return eval(node->value, env);

        case ast_type::ast_assign:
            return eval_assign(node, env);

        case ast_type::ast_compound:
            return eval_scope(node, env);

        case ast_type::ast_num_expr:
            return new rt_value(node->number);

        case ast_type::ast_string_expr:
            return new rt_value(node->symbol);

        case ast_type::ast_function:
            return eval_function(node, env);

        case ast_type::ast_call:
            return eval_call(node, env);

        case ast_type::ast_array:
            return eval_array(node, env);

        case ast_type::ast_object:
            return eval_object(node, env);

        case ast_type::ast_member:
            return eval_member(node, env);

        case ast_type::ast_noop:
            return new rt_value();

        case ast_type::ast_arrindex:
            return eval_arrindex(node, env);

        case ast_type::ast_import:
            return eval_import(node, env);

        case ast_type::ast_binop:
            return eval_binary(node, env);

        case ast_type::ast_if:
            return eval_if(node, env);

        case ast_type::ast_while:
            return eval_while(node, env);
    }
    return new rt_value();
}

rt_value_t* interpreter::eval_assign(ast_node* node, environment_t* env)
{
    rt_value* value = eval(node->value, env);
    if (value->type != node->data_type) error(string_format("expected type %s for %s, got %s", dtype_to_str(node->data_type).c_str(), node->symbol.c_str(), dtype_to_str(value->type).c_str()), node->pos, source).spit();
    env->assign(node->symbol, value);
    return new rt_value();
}

rt_value_t* interpreter::eval_object(ast_node* node, environment_t* env) {
    std::map<std::string, rt_value*> object;

    for (auto elem : node->children) {
        object[elem->symbol] = eval(elem->value, env);
    }

    return new rt_value(object);
}

rt_value_t* interpreter::eval_array(ast_node* node, environment_t* env)
{
    std::vector<rt_value*> arr;

    for (auto elem : node->children) {
        arr.push_back(eval(elem, env));
    }

    return new rt_value(arr);
}

rt_value_t* interpreter::eval_function(ast_node* node, environment_t* env)
{
    rt_value_t* fc = new rt_value(node->value, node);
    return fc;
}

rt_value_t* interpreter::eval_scope(ast_node* node, environment_t* env)
{
    rt_value_t* rt_val;
    environment_t* s = new environment(env);

    for (ast_node* elem : node->children) {
        if (elem->type == ast_type::ast_return) rt_val = eval(elem, s);
        else eval(elem, s);
    }

    return rt_val;
}

rt_value_t* interpreter::eval_scope_samenv(ast_node* node, environment_t* env)
{
    rt_value_t* rt_val;

    for (ast_node* elem : node->children) {
        if (elem->type == ast_type::ast_return) rt_val = eval(elem, env);
        else eval(elem, env);
    }

    return rt_val;
}

// rt_value_t* interpreter::eval_call(ast_node* node, environment_t* env)
// {
//     rt_value_t* scope = env->get_var(node->symbol);
//     environment_t* cenv = new environment(env);
//     std::vector<rt_value*> args;

//     for (int i = 0; i < scope->proto->children.size(); i++) {
//         ast_node* arg = node->value->children[i];
//         if (arg == nullptr) break;
//         ast_node* id = scope->proto->children[i];
//         if (id == nullptr) break;
//         rt_value* evaluated = eval(arg, env);
//         if (evaluated->type != id->data_type && scope->proto->data_type != dtype::cfunction) error(string_format("expected type %s for argument %s, got %s", dtype_to_str(id->data_type).c_str(), id->symbol.c_str(), dtype_to_str(evaluated->type).c_str()), node->pos, source).spit();
//         args.push_back(evaluated);
//         cenv->assign(id->symbol, evaluated);
//     }

//     rt_value_t* rt_val;
//     dtype_t ftype = scope->proto->data_type;

//     if (ftype != dtype::cfunction) {
//         std::vector<ast_node*> body = scope->body->children;
//         if (body.size() > 0) {
//             for (ast_node* elem : body) {
//                 if (elem->type == ast_type::ast_return) rt_val = eval(elem, cenv);
//                 else eval(elem, cenv);
//             }
//         }
//     } else {
//         rt_val = scope->cfunc(args, env);
//     }

//     return rt_val;
// }

rt_value_t* interpreter::eval_call(ast_node* node, environment_t* env)
{
    rt_value_t* scope = env->get_var(node->symbol);

    if (scope && scope->type == dtype::func || scope->type == dtype::cfunction) {
        environment_t* cenv = new environment(env);
        std::vector<rt_value*> args;

        for (int i = 0; i < scope->proto->children.size(); i++) {
            ast_node* arg = node->value->children[i];
            if (arg == nullptr) break;
            ast_node* id = scope->proto->children[i];
            if (id == nullptr) break;
            rt_value* evaluated = eval(arg, env);
            if (evaluated->type != id->data_type && scope->type != dtype::cfunction) {
                error(string_format("expected type %s for argument %s, got %s", dtype_to_str(id->data_type).c_str(), id->symbol.c_str(), dtype_to_str(evaluated->type).c_str()), node->pos, source).spit();
                return nullptr;
            }
            args.push_back(evaluated);
            cenv->assign(id->symbol, evaluated);
        }

        rt_value_t* rt_val;
        dtype_t ftype = scope->proto->data_type;

        if (ftype != dtype::cfunction) {
            std::vector<ast_node*> body = scope->body->children;
            if (body.size() > 0) {
                for (ast_node* elem : body) {
                    if (elem->type == ast_type::ast_return) rt_val = eval(elem, cenv);
                    else eval(elem, cenv);
                }
            }
        } else {
            rt_val = scope->cfunc(args, env);
        }

        return rt_val;
    } else {
        // Handle the case where the function is not found
        error("function not found: " + node->symbol, node->pos, source).spit();
        return nullptr;
    }
}

// rt_value_t* interpreter::call_func(rt_value* func, std::vector<rt_value*> args, environment_t* env)
// {
//     environment_t* cenv = new environment(env);
//     for (int i = 0; i < func->proto->children.size(); i++) {
//         ast_node* id = func->proto->children[i];
//         //print_node(func->proto);
//         cenv->assign(id->symbol, args[i]);
//     }

//     rt_value_t* rt_val;
//     std::vector<ast_node*> body = func->body->children;
//     if (body.size() > 0) {
//         for (ast_node* elem : body) {
//             if (elem->type == ast_type::ast_return) rt_val = eval(elem, cenv);
//             else eval(elem, cenv);
//         }
//     }

//     return rt_val;
// }

rt_value_t* interpreter::call_func(rt_value* func, std::vector<rt_value*> args, environment_t* env)
{
    environment_t* cenv = new environment(env);

    // Check if func and func->proto have valid elements
    if (!func || !func->proto) {
        error("invalid function or function prototype", func ? func->body->pos : position(), source).spit();
        return nullptr;
    }

    // Check if args size matches the number of parameters in func->proto
    if (args.size() != func->proto->children.size()) {
        error("mismatched number of arguments and function parameters", func->proto->pos, source).spit();
        delete cenv;  // Clean up allocated environment
        return nullptr;
    }

    // Assign arguments to parameters in the new environment
    for (int i = 0; i < args.size(); i++) {
        ast_node* id = func->proto->children[i];
        if (!id) {
            error("invalid function parameter", func->proto->pos, source).spit();
            delete cenv;  // Clean up allocated environment
            return nullptr;
        }
        cenv->assign(id->symbol, args[i]);
    }

    rt_value_t* rt_val = nullptr;  // Initialize to nullptr

    // Evaluate the function body
    std::vector<ast_node*> body = func->body->children;
    if (body.size() > 0) {
        for (ast_node* elem : body) {
            if (elem->type == ast_type::ast_return) rt_val = eval(elem, cenv);
            else eval(elem, cenv);
        }
    }

    delete cenv;  // Clean up allocated environment

    return rt_val;
}


rt_value_t* interpreter::eval_call(ast_node* node, environment_t* env, rt_value* func)
{
    rt_value_t* scope = func;
    environment_t* cenv = new environment(env);
    std::vector<rt_value*> args;

    for (int i = 0; i < scope->proto->children.size(); i++) {
        if (node->value->children.size() <= i && scope->proto->data_type != dtype::cfunction) error(string_format("expected %d args, got %d", scope->proto->children.size(), node->value->children.size()), node->pos, source).spit();
        ast_node* arg = node->value->children[i];
        if (arg == nullptr) break;
        ast_node* id = scope->proto->children[i];
        if (id == nullptr) break;
        rt_value* evaluated = eval(arg, env);
        if (evaluated->type != id->data_type && scope->proto->data_type != dtype::cfunction) error(string_format("expected type %s for argument %s, got %s", dtype_to_str(id->data_type).c_str(), id->symbol.c_str(), dtype_to_str(evaluated->type).c_str()), node->pos, source).spit();
        args.push_back(evaluated);
        cenv->assign(id->symbol, evaluated);
    }

    rt_value_t* rt_val;
    dtype_t ftype = scope->proto->data_type;

    if (ftype != dtype::cfunction) {
        std::vector<ast_node*> body = scope->body->children;
        if (body.size() > 0) {
            for (ast_node* elem : body) {
                if (elem->type == ast_type::ast_return) rt_val = eval(elem, cenv);
                else eval(elem, cenv);
            }
        }
    } else {
        rt_val = scope->cfunc(args, env);
    }

    return rt_val;
}

// rt_value_t* interpreter::eval_member(ast_node* node, environment_t* env) {
//     // Get the left-hand side symbol directly
//     std::string member_name = node->symbol;

//     // Retrieve the object from the environment
//     rt_value_t* obj = env->get_var(member_name);

//     if (obj->type == dtype::object) {
//         if (obj->children.find(member_name) != obj->children.end()) {
//             if (node->value->type == ast_type::ast_identifier) return obj->children[node->value->symbol];
//             else {
//                 ast_node* last_node = node->value;
//                 rt_value* last_value = obj->children[member_name];

//                 while (last_node->type == ast_type::ast_member) {
//                     printf("%s\n", last_node->symbol.c_str());
//                     print_node(last_node);
//                     last_node = last_node->value;
//                     print_node(last_node);
//                     printf("%s, %s\n", last_value->ts().c_str(), last_node->symbol.c_str());
//                     last_value = obj->children[last_node->symbol];
//                 }

//                 return last_value;
//             }
//         } else {
//             // Handle member not found error
//             return new rt_value("not found");
//         }
//     } else {
//         // Handle error when trying to access a member on a non-object
//         return new rt_value("not an object");
//     }
//     return new rt_value();
// }

// rt_value_t* interpreter::eval_member(ast_node* node, environment_t* env) {
//     // Get the left-hand side symbol directly
//     std::string member_name = node->symbol;

//     // Retrieve the object from the environment
//     rt_value_t* obj = env->get_var(member_name);

//     if (obj->type == dtype::object) {
//         if (node->value->type == ast_type::ast_identifier) {
//             // Single member access, return the corresponding value
//             std::string member_symbol = node->value->symbol;
//             if (obj->children.find(member_symbol) != obj->children.end()) {
//                 return obj->children[member_symbol];
//             } else {
//                 // Handle member not found error
//                 //return new rt_value("Member '" + member_symbol + "' not found.");
//                 error(string_format("member %s not found", member_symbol.c_str()), node->value->pos, source).spit();
//             }
//         } else {
//             // Traverse member expressions
//             ast_node* current_node = node->value;

//             while (current_node->type == ast_type::ast_member) {
//                 std::string current_member_symbol = current_node->symbol;
//                 if (obj->children.find(current_member_symbol) != obj->children.end()) {
//                     obj = obj->children[current_member_symbol];
//                     current_node = current_node->value;
//                 } else {
//                     // Handle member not found error
//                     //return new rt_value("Member '" + current_member_symbol + "' not found.");
//                     error(string_format("member %s not found", current_member_symbol.c_str()), current_node->pos, source).spit();
//                 }
//             }

//             // Evaluate the final member access
//             if (current_node->type == ast_type::ast_identifier) {
//                 std::string final_member_symbol = current_node->symbol;
//                 if (obj->children.find(final_member_symbol) != obj->children.end()) {
//                     return obj->children[final_member_symbol];
//                 } else {
//                     // Handle member not found error
//                     //return new rt_value("Member '" + final_member_symbol + "' not found.");
//                     error(string_format("member %s not found", final_member_symbol.c_str()), current_node->pos, source).spit();
//                 }
//             }
//         }
//     } else {
//         // Handle error when trying to access a member on a non-object
//         //return new rt_value("Not an object.");
//         error(string_format("not an object"), node->pos, source).spit();
//     }
    
//     return new rt_value();
// }

rt_value_t* interpreter::eval_member(ast_node* node, environment_t* env) {
    // Get the left-hand side symbol directly
    std::string member_name = node->symbol;

    // Retrieve the object from the environment
    rt_value_t* obj = env->get_var(member_name);

    if (obj->type == dtype::object) {
        if (node->value->type == ast_type::ast_identifier) {
            // Single member access, return the corresponding value
            std::string member_symbol = node->value->symbol;
            if (obj->children.find(member_symbol) != obj->children.end()) {
                return obj->children[member_symbol];
            } else {
                // Handle member not found error
                error(string_format("member %s not found", member_symbol.c_str()), node->value->pos, source).spit();
            }
        } else {
            // Traverse member expressions
            ast_node* current_node = node->value;

            while (current_node->type == ast_type::ast_member) {
                std::string current_member_symbol = current_node->symbol;
                if (obj->children.find(current_member_symbol) != obj->children.end()) {
                    obj = obj->children[current_member_symbol];
                    current_node = current_node->value;
                    
                    if (current_node->type == ast_type::ast_call) {
                        obj = obj->children[current_node->symbol];
                        return eval_call(current_node, env, obj);
                    }
                } else {
                    // Handle member not found error
                    error(string_format("member %s not found", current_member_symbol.c_str()), current_node->pos, source).spit();
                }
            }

            // Evaluate the final member access
            if (current_node->type == ast_type::ast_identifier) {
                std::string final_member_symbol = current_node->symbol;
                if (obj->children.find(final_member_symbol) != obj->children.end()) {
                    return obj->children[final_member_symbol];
                } else {
                    // Handle member not found error
                    error(string_format("member %s not found", final_member_symbol.c_str()), current_node->pos, source).spit();
                }
            }

            if (current_node->type == ast_type::ast_call) {
                obj = obj->children[current_node->symbol];
                return eval_call(current_node, env, obj);
            }
        }
    } else {
        // Handle error when trying to access a member on a non-object
        error(string_format("not an object"), node->pos, source).spit();
    }
    
    return new rt_value();
}

rt_value_t* interpreter::eval_arrindex(ast_node* node, environment_t* env)
{
    rt_value_t* arr = env->get_var(node->symbol);
    if (arr->type != dtype::array) {
        if (arr->type != dtype::object) error(string_format("not an array or object"), node->pos, source).spit();
        rt_value_t* idx = eval(node->value, env);
        if (idx->type != dtype::string) error(string_format("not an indexable type for object"), node->pos, source).spit();
        return arr->children[idx->str];
    }

    if (node->value->type == ast_type::ast_num_expr) {
        return arr->arr[node->value->number];
    } else {
        rt_value_t* idx = eval(node->value, env);
        if (idx->type != dtype::integer) error(string_format("not an indexable type for array"), node->pos, source).spit();
        return arr->arr[idx->num];
    }
}

rt_value_t* interpreter::eval_import(ast_node* node, environment_t* env)
{
    ast_node* path = node->value;
    ast_node* id = node->svalue;

    rt_value* strpath = eval(path, env);

    if (strpath->type == dtype::string) {
        std::string contents = futil::read_file(strpath->str.c_str());
        interpreter_t i(contents);
        rt_value* res = i.run();
        env->assign(id->symbol, res);
    } else {
        error("invalid arguments to import", path->pos, source).spit();
    }

    return new rt_value();
}

rt_value_t* interpreter::eval_binary(ast_node* node, environment_t* env)
{
    rt_value* left = eval(node->value, env);
    rt_value* right = eval(node->svalue, env);
    ///print_node(node->svalue);

    if (left->type == dtype::integer && right->type == dtype::integer) {
        std::string op = node->symbol;

        if (op == "+") return new rt_value(left->num + right->num);
        if (op == "-") return new rt_value(left->num - right->num);
        if (op == "/") return new rt_value(left->num / right->num);
        if (op == "*") return new rt_value(left->num * right->num);
        if (op == "==") return new rt_value(left->num == right->num);
        if (op == ">=") return new rt_value(left->num >= right->num);
        if (op == "<=") return new rt_value(left->num <= right->num);
        if (op == "<") return new rt_value(left->num < right->num);
        if (op == ">") return new rt_value(left->num > right->num);
    }

    if (left->type == dtype::string && right->type == dtype::string) {
        std::string op = node->symbol;

        if (op == "+") return new rt_value(left->str + right->str);
        if (op == "-") error("cannot sub string by string", node->pos, source).spit();
        if (op == "/") error("cannot divide string by string", node->pos, source).spit();
        if (op == "*") error("cannot multiply string by string", node->pos, source).spit();
        if (op == "==") return new rt_value(left->str == right->str);
        if (op == ">=") error("cannot check if string is greater than or equal to string", node->pos, source).spit();
        if (op == "<=") error("cannot check if string is less than or equal to string", node->pos, source).spit();
        if (op == "<") error("cannot check if string is less than string", node->pos, source).spit();
        if (op == ">") error("cannot check if string is greater than string", node->pos, source).spit();
    }

    if (left->type == dtype::string && right->type == dtype::integer) {
        std::string op = node->symbol;

        if (op == "+") return new rt_value(left->str + std::to_string(right->num));
        if (op == "-") error("cannot sub string by number", node->pos, source).spit();
        if (op == "/") error("cannot divide string by number", node->pos, source).spit();
        if (op == "*") return new rt_value(repeat(left->str, (int)right->num));
        if (op == ">=") error("cannot check if string is greater than or equal to number", node->pos, source).spit();
        if (op == "<=") error("cannot check if string is less than or equal to number", node->pos, source).spit();
        if (op == "<") error("cannot check if string is less than number", node->pos, source).spit();
        if (op == ">") error("cannot check if string is greater than number", node->pos, source).spit();
    }

    return new rt_value();
}

rt_value_t* interpreter::eval_if(ast_node* node, environment_t* env)
{
    rt_value* evaluated = eval(node->svalue, env);
    if (evaluated->type == dtype::boolean) {
        if (evaluated->boolean == true) {
            eval_scope_samenv(node->value, env);
        }
    } else {
        if (evaluated->type != dtype::nil) {
            eval_scope_samenv(node->value, env);
        }
    }

    return new rt_value();
}

rt_value_t* interpreter::eval_while(ast_node* node, environment_t* env)
{
    rt_value* evaluated = eval(node->svalue, env);
    if (evaluated->type == dtype::boolean) {
        while (evaluated->boolean == true) {
            eval_scope_samenv(node->value, env);
            evaluated = eval(node->svalue, env);
        }
    } else {
        while (evaluated->type != dtype::nil) {
            eval_scope_samenv(node->value, env);
            evaluated = eval(node->svalue, env);
        }
    }

    return new rt_value();
}