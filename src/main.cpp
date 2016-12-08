#include <iostream>
#include <fstream>
#include <string>
#include <streambuf>
#include <utility>
#include "lexer.h"

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

    Lexer lexer(std::move(content));

    Token token;
    try {
        while ((token = lexer.get_token()).type != T_EOF) {
            cout << token << endl;
        }
    } catch (std::exception e) {
        cout << e.what() << endl;
        return 1;
    }

    fin.close();
}
