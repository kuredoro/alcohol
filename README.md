## `alcohol` : a memory management linter for C

###### 🚧 Work in progress 🚧

### How to run?

For C
```bash
$ clang -cc1 -load build/libalcohol.so -plugin alcohol test.cpp
```

and C++
```bash
$ clang++ -c -Xclang -load -Xclang build/libalcohol.so -Xclang -plugin -Xclang alcohol test.cpp
```
