#ifndef FUTIL_H_
#define FUTIL_H_

#include <fstream>
#include <string>

namespace futil {
    inline std::string read_file(const char* path) {
        std::string fin, line;
        std::ifstream f(path);

        if (f.is_open()) {
            while (std::getline(f, line)) {
                fin += line + "\n";
            }

            f.close();
        }

        return fin;
    }

    inline void write_file(const char* path, std::string txt) {
        std::ofstream f;
        f.open(path);
        f << txt;
        f.close();
    }
};

#endif // FUTIL_H_