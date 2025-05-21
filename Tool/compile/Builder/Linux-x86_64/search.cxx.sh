echo "Compiling: search.cxx"
g++ -c -o Temp/Object/search.cxx.o \
    Source/search.cxx    \
    -O3 -Ofast -Os -s      \
    -march=native          \
    -Wall -Wextra -pedantic -Werror  -I Source/ -std=c++23