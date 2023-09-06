#include "ast.h"
// #include "cpp_front.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "runtime.h"
#include "token.h"
#include <cstdio>
#include <cstdlib>
#include <error.h>
#include <futil.h>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 2) {
        argv[1] = "examples/basic.du";
    }

    std::string fcontents = futil::read_file(argv[1]);
    // parser_t pars = parser(fcontents);
    // ast_node* idk = pars.parse();
    //printf("%s\n", asm_res.c_str());
    //print_node(idk);

    interpreter_t inter = interpreter(fcontents);
    rt_value_t* eval = inter.run();
    //eval->out();

    // std::string ccode = cpp_frontend::from_root(idk);
    // futil::write_file(argv[2], ccode);

    return EXIT_SUCCESS;
}
