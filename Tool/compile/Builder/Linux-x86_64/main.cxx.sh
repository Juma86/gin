echo "Compiling: main.cxx"

g++ -c -o Temp/Object/main.cxx.o \
    Source/main.cxx            \
    -O3 -Ofast -Os -s          \
    -march=native \
    -Wall -Wextra -pedantic -Werror -I Source/ -std=c++20