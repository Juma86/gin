echo "Compiling: download.cxx"
g++ -c -o Temp/Object/download.cxx.o \
    Source/download.cxx            \
    -O3 -Ofast -Os -s          \
    -march=native \
    -Wall -Wextra -pedantic -Werror \
    -std=c++20 -I Source/