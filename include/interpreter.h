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
    rt_value_t* eval_call(ast_node* node, environment_t* env, rt_value* func);
    rt_value_t* eval_member(ast_node* node, environment_t* env);
} interpreter_t;

#endif // __INTERPRETER_H__