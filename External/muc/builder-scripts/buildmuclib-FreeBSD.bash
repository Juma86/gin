#g++15 -shared -fPIC -Wl,-soname,libmuc.so -Wall -Wextra -Werror ./muclib.cxx -o ./libmuc.so

echo "Building muc shared object ..."

# build muc library object
g++15 -c -fpic -Wall -Wextra -Werror -O3 -Os -s -Ofast \
    -o ./../objects/muclib.o ./../source/muclib.cxx

# build muc dynamic library
g++15 -shared \
    -o ./../binaries/libmuc.so ./../objects/muclib.o
