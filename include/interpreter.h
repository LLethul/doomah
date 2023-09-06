#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#include "ast.h"
#include "env.h"
#include "parser.h"
#include "runtime.h"
#include "types.h"

typedef struct interpreter {
    std::string source;
    parser_t p;

    interpreter(std::string source) : source(source), p(source) {};

    rt_value_t* run();
    
    rt_value_t* eval(ast_node* node, environment_t* env);
    rt_value_t* eval_assign(ast_node* node, environment_t* env);
    rt_value_t* eval_object(ast_node* node, environment_t* env);
    rt_value_t* eval_array(ast_node* node, environment_t* env);
    rt_value_t* eval_function(ast_node* node, environment_t* env);
    rt_value_t* eval_scope(ast_node* node, environment_t* env);
    rt_value_t* eval_call(ast_node* node, environment_t* env);
    rt_value* call_func(rt_value* func, std::vector<rt_value*> args, environment_t* env);
    rt_value_t* eval_call(ast_node* node, environment_t* env, rt_value* func);
    rt_value_t* eval_cfunc(rt_value* cfunc);
    rt_value_t* eval_member(ast_node* node, environment_t* env);
    rt_value_t* eval_arrindex(ast_node* node, environment_t* env);
    rt_value_t* eval_import(ast_node* node, environment_t* env);
    rt_value_t* eval_binary(ast_node* node, environment_t* env);
    rt_value_t* eval_if(ast_node* node, environment_t* env);
    rt_value_t* eval_while(ast_node* node, environment_t* env);
    rt_value_t* eval_scope_samenv(ast_node* node, environment_t* env);
} interpreter_t;

#endif // __INTERPRETER_H__