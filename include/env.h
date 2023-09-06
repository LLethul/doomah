#ifndef __ENV_H__
#define __ENV_H__

#include <map>
#include <utility>
#include "runtime.h"

typedef struct environment {
    std::shared_ptr<environment> parent;
    std::map<std::string, rt_value*> variables;
    void* interpret;

    environment(environment* parent = nullptr) : parent(parent) {}

    void assign(const std::string& key, rt_value* val) {
        variables[key] = val;
    }

    rt_value* get_var(const std::string& key) {
        // Check if the variable exists in the current environment
        auto it = variables.find(key);
        if (it != variables.end()) {
            return it->second;
        }

        // If not found in the current environment, check in the parent environment
        if (parent != nullptr) {
            return parent->get_var(key);
        }

        // Return nullptr if the variable is not found
        return nullptr; // Changed to return nullptr for better error handling
    }

    void* get_interpreter() {
        if (interpret == nullptr && parent != nullptr) return parent->get_interpreter();
        else {
            if (interpret != nullptr) return interpret;
            return nullptr;
        }
    }
} environment_t;

#endif // __ENV_H__