#!/usr/bin/sh
if [ "$1" = "clang" ]
then
    clang -cc1 -load build/lib/libalcohol.so -plugin alcohol -fcolor-diagnostics "$2"
elif [ "$1" = "clang++" ]
then
    clang++ -c -Xclang -load -Xclang build/lib/libalcohol.so -Xclang -plugin -Xclang alcohol "$2"
elif [ "$1" = "clang++debug" ]
then
    clang++ -c -Xclang -load -Xclang build/lib/libalcohol.so -Xclang -plugin -Xclang alcohol -Xclang -plugin-arg-alcohol -Xclang -debug "$2"
else
    echo "Usage: alcohol.sh [clang | clang++] file"
fi
