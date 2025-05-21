echo "Compiling: fs.cxx"
g++ -c -o Temp/Object/fs.cxx.o \
    Source/aystl/fs.cxx    \
    -O3 -Ofast -Os -s      \
    -march=native          \
    -Wall -Wextra -pedantic -Werror  -I Source/