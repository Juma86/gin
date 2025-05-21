echo "Compiling: tests.cxx"
g++ -c -o Temp/Object/tests.cxx.o \
    Source/tests.cxx         \
    -O3 -Ofast -Os -s              \
    -march=native                  \
    -Wall -Wextra -pedantic -Werror -I Source/ -std=c++20