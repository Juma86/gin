echo "Compiling: cmd.cxx"
g++ -c -o Temp/Object/cmd.cxx.o \
    Source/aystl/cmd.cxx    \
    -O3 -Ofast -Os -s      \
    -march=native          \
    -Wall -Wextra -pedantic -Werror -std=c++23 -I Source/