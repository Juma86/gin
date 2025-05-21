echo "Compiling: dbr.cxx"
g++ -c -o Temp/Object/dbr.cxx.o \
    Source/aystl/dbr.cxx    \
    -O3 -Ofast -Os -s      \
    -march=native          \
    -Wall -Wextra -pedantic -Werror  -I Source/ -std=c++20