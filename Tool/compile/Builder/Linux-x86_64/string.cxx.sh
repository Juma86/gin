echo "Compiling: sftp.cxx"
g++ -c -o Temp/Object/string.cxx.o \
    Source/aystl/string.cxx   \
    -O3 -Ofast -Os -s         \
    -march=native             \
    -Wall -Wextra -pedantic -Werror  -I Source/ -std=c++20