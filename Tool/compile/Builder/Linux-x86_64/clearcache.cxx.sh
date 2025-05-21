echo "Compiling: clearcache.cxx"
g++ -c -o Temp/Object/clearcache.cxx.o \
    Source/clearcache.cxx            \
    -O3 -Ofast -Os -s          \
    -march=native \
    -Wall -Wextra -pedantic -Werror \
    -std=c++20 -I Source/