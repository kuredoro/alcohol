#!/usr/bin/sh
if [ "$1" = "clang" ]
then
    clang -cc1 -load build/libalcohol.so -plugin hello-world "$2"
elif [ "$1" = "clang++" ]
then
    clang++ -c -Xclang -load -Xclang build/libalcohol.so -Xclang -plugin -Xclang hello-world "$2"
else
    echo "Usage: alcohol.sh [clang | clang++] file"
fi
