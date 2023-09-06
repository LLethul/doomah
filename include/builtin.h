#ifndef __BUILTIN_H__
#define __BUILTIN_H__

#include "env.h"
#include "futil.h"
#include "interpreter.h"
#include "runtime.h"
#include "types.h"
#include <string>
#include <vector>

inline rt_value* string_concat(std::vector<rt_value*> args, void* env) {
    return new rt_value(args[0]->str + args[1]->str);
}

inline rt_value* string_to_string(std::vector<rt_value*> args, void* env) {
    return new rt_value(args[0]->ts());
}

inline rt_value* array_push(std::vector<rt_value*> args, void* env) {
    args[0]->arr.push_back(args[1]);
    return args[1];
}

inline rt_value* array_remove(std::vector<rt_value*> args, void* env) {
    args[0]->arr.erase(args[0]->arr.begin() + args[1]->num);
    return new rt_value();
}

inline rt_value* array_pop(std::vector<rt_value*> args, void* env) {
    rt_value* saved = *(args[0]->arr.begin());
    args[0]->arr.erase(args[0]->arr.begin() + 0);
    return saved;
}

inline rt_value* array_foreach(std::vector<rt_value*> args, void* env) {
    rt_value* arr = args[0];
    rt_value* func = args[1];
    environment_t* env_cast = static_cast<environment*>(env);
    interpreter_t* inter =  static_cast<interpreter*>(env_cast->get_interpreter());

    for (rt_value* idx : arr->arr) {
        inter->call_func(func, {idx}, env_cast);
    }

    return new rt_value();
}

inline void def_on_env(environment_t* env) {
    // std::map<std::string, rt_value*> stringbase = {
    //     {"string", new rt_value(std::map<std::string, rt_value*>{
    //         {"concat", new rt_value(string_concat)}
    //     })}
    // };

    rt_value* sbase = new rt_value(std::map<std::string, rt_value*>{
        {"concat", new rt_value((std::function<rt_value*(std::vector<rt_value*>, void*)>)string_concat)},
        {"to_string", new rt_value((std::function<rt_value*(std::vector<rt_value*>, void*)>)string_to_string)}
    });

    // std::map<std::string, rt_value*> arrbase = {
    //     {"array", new rt_value(std::map<std::string, rt_value*>{
    //         {"push", new rt_value(array_push)}
    //     })}
    // };

    rt_value* abase =  new rt_value(std::map<std::string, rt_value*>{
        {"push", new rt_value((std::function<rt_value*(std::vector<rt_value*>, void*)>)array_push)},
        {"remove", new rt_value((std::function<rt_value*(std::vector<rt_value*>, void*)>)array_remove)},
        {"pop", new rt_value((std::function<rt_value*(std::vector<rt_value*>, void*)>)array_pop)},
        {"foreach", new rt_value((std::function<rt_value*(std::vector<rt_value*>, void*)>)array_foreach)}
    });

    env->assign("string", sbase);
    env->assign("array", abase);
}

#endif // __BUILTIN_H__