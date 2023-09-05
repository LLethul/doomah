#ifndef ERROR_H_
#define ERROR_H_

#include "position.h"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <stdio.h>

inline std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

typedef struct error {
    std::string what;
    position_t position;
    std::string source;
    bool has_pos;

    error(std::string w) : what(w), has_pos(false) {}
    error(std::string w, position_t pos, std::string src) : what(w), has_pos(true), source(src), position(pos) {};

    inline void spit() {
        if (has_pos) {
            std::string line = split(source, "\n")[position.ln - 1];
            std::string arrow = std::string(position.col - 1, ' ') + "^";
            printf("%s\n%s\n", line.c_str(), arrow.c_str());
            printf("\nerror: %s\n", what.c_str());
        } else printf("error: %s\n", what.c_str());

        exit(EXIT_FAILURE);
    }
} error_t;

namespace error_util {
    inline void spit(std::string what) {
        error(what).spit();
    }
}

#endif // ERROR_H_