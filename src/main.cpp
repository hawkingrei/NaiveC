#include <iostream>
#include <fstream>
#include <string>
#include <streambuf>
#include <utility>
#include "lexer.h"
#include "parser.h"
#include "generator.h"

using std::cout;
using std::endl;
using std::string;

int main(int argc, char *argv[]) {
    if (argc == 1) {
        cout << "no input file" << endl;
        return 1;
    }

    string filename(argv[1]);

    std::ifstream fin(filename);

    if (!fin) {
        cout << "invalid filename" << endl;
        return 1;
    }

    std::string content((std::istreambuf_iterator<char>(fin)),
                        std::istreambuf_iterator<char>());

    fin.close();

    Lexer lexer(std::move(content));

    Parser parser(lexer.tokenize());

    parser.main_loop();

    std::string file_base = filename.substr(0, filename.find_last_of("."));

    Generator::instance()->dump(file_base + ".bc");
}
