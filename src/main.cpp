#include <iostream>
#include <fstream>
#include <string>
#include <streambuf>

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

    cout << content << endl;

    fin.close();
}
