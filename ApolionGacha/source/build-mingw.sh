#!/bin/sh
# Run from this directory. Requires MinGW-w64.
set -eu
x86_64-w64-mingw32-windres resources.rc -O coff -o resources.o
x86_64-w64-mingw32-g++ main.cpp resources.o -std=c++17 -O2 -municode -mwindows -static -static-libgcc -static-libstdc++ -lgdiplus -lcomctl32 -lole32 -luuid -lshell32 -lcomdlg32 -lpropsys -lbcrypt -o ../ApolionGacha.exe
rm resources.o
