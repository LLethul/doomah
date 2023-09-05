#ifndef TYPES_H_
#define TYPES_H_

#include <string>
typedef enum struct dtype {
    integer,
    string,
    func,
    object,
    array,
    nil,
    cfunction,
} dtype_t;

inline dtype_t str_to_dtype(std::string dt) {
    if (dt == "int") {
        return dtype::integer;
    }

    if (dt == "str") {
        return dtype::string;
    }

    if (dt == "func") {
        return dtype::func;
    }

    if (dt == "object") {
        return dtype::object;
    }

    if (dt == "array") {
        return dtype::array;
    }

    if (dt == "none") {
        return dtype::array;
    }

    return dtype::nil;
}

inline std::string dtype_to_str(dtype_t dt) {
    switch (dt) {
        case dtype::integer:
            return "int";

        case dtype::string:
            return "string";

        case dtype::func:
            return "function";

        case dtype::array:
            return "array";

        case dtype::object:
            return "object";

        case dtype::nil:
            return "nil";

        case dtype::cfunction:
            return "cfunc";
    }
}

#endif // TYPES_H_