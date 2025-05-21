echo "Compiling: ginutils.cxx"
g++ -c -o Temp/Object/ginutils.cxx.o \
    Source/gin/ginutils.cxx    \
    -O3 -Ofast -Os -s      \
    -march=native          \
    -Wall -Wextra -pedantic -Werror  -I Source/ -std=c++20