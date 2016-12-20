# NaiveC

Naive C compiler

## Environment

* LLVM 3.9.0
* Clang
* CMake

## Usage

```bash
cmake [-DCMAKE_INSTALL_PREFIX="/tmp/llvm"] .
make
./naive_c tests/kmp.c # run compiler, ir will output to stderr, bc will write to file
lli tests/kmp.bc # interpret bytecode
llc tests/kmp.bc # compile to assembly language
```

You should ensure LLVM in your path so that cmake can find it, or you can run cmake with `-D CMAKE_INSTALL_PREFIX` option.
