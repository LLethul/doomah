#include "interpreter.h"
#include "ast.h"
#include "env.h"
#include "parser.h"
#include "types.h"
#include <error.h>
#include <vector>
#include <iostream>

rt_value* print(std::vector<rt_value*> args) {
    std::string fin;
    for (rt_value* param : args) {
        fin += param->ts()+" ";
    }

    printf("%s\n", fin.c_str());
    return new rt_value();
}

#include <functional>
#include <type_traits>

rt_value_t* interpreter::run()
{
    rt_value_t* rt_val;
    environment_t* scope = new environment();

    // rlly hacky but
    rt_value* arg0;

    scope->assign("print", new rt_value(print));
    std::function<rt_value*(std::vector<rt_value*>)> cfunc = print;

    ast_node* root = p.parse();
    print_node(root);
    rt_val = eval(root, scope);

    return rt_val;
}

rt_value_t* interpreter::eval(ast_node* node, environment_t* env)
{
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
    }
    return new rt_value();
}

rt_value_t* interpreter::eval_assign(ast_node* node, environment_t* env)
{
    rt_value* value = eval(node->value, env);
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

rt_value_t* interpreter::eval_call(ast_node* node, environment_t* env)
{
    rt_value_t* scope = env->get_var(node->symbol);
    environment_t* cenv = new environment(env);

    for (int i = 0; i < scope->proto->children.size(); i++) {
        ast_node* arg = node->value->children[i];
        ast_node* id = scope->proto->children[i];
        cenv->assign(id->symbol, eval(arg, env));
    }

    rt_value_t* rt_val;
    std::vector<ast_node*> body = scope->body->children;

    if (body.size() > 0) {
        for (ast_node* elem : body) {
            if (elem->type == ast_type::ast_return) rt_val = eval(elem, cenv);
            else eval(elem, cenv);
        }
    }

    return rt_val;
}

rt_value_t* interpreter::eval_call(ast_node* node, environment_t* env, rt_value* func)
{
    rt_value_t* scope = func;
    environment_t* cenv = new environment(env);
    std::vector<rt_value*> args;

    for (int i = 0; i < scope->proto->children.size(); i++) {
        ast_node* arg = node->value->children[i];
        if (arg == nullptr) break;
        ast_node* id = scope->proto->children[i];
        if (id == nullptr) break;
        rt_value* evaluated = eval(arg, env);
        args.push_back(evaluated);
        cenv->assign(id->symbol, evaluated);
    }

    rt_value_t* rt_val;
    std::vector<ast_node*> body = scope->body->children;
    dtype_t ftype = scope->proto->data_type;

    if (ftype != dtype::cfunction) {
        if (body.size() > 0) {
            for (ast_node* elem : body) {
                if (elem->type == ast_type::ast_return) rt_val = eval(elem, cenv);
                else eval(elem, cenv);
            }
        }
    } else {
        rt_val = scope->cfunc(args);
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