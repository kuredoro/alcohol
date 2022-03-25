## `alcohol` : a memory management linter for C

###### 🚧 Work in progress 🚧

### How to build?

The project uses conan package manager and C++17 standard.

To configure the project
```bash
$ cd alcohol
$ conan install -if build --build=missing .
$ cmake -S . -B build
$ cmake --build build -j 8
```

When building on Windows, you need to specify the desired configuration in the last line:
```bash
$ cmake --build build --config Release
```

### How to run?

For C
```bash
$ clang -cc1 -load build/libalcohol.so -plugin alcohol test.cpp
```

and C++
```bash
$ clang++ -c -Xclang -load -Xclang build/libalcohol.so -Xclang -plugin -Xclang alcohol test.cpp
```
