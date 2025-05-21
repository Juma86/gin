#!/bin/bash
#g++ -shared -fPIC -Wl,-soname,libmuc.so -Wall -Wextra -Werror ./muclib.cxx -o ./libmuc.so

echo "Building muc shared object ..."

# build muc library object
g++ -c -fpic -Wall -Wextra -Werror -O3 -Os -s -Ofast -lssh \
    -o ./../objects/muclib.o ./../source/muclib.cxx

# build muc dynamic library
g++ -shared \
    -o ./../binaries/libmuc.so ./../objects/muclib.o
