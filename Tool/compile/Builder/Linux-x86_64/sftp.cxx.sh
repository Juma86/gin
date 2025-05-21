echo "Compiling: sftp.cxx"
g++ -c -o Temp/Object/sftp.cxx.o \
    Source/aystl/sftp.cxx    \
    -O3 -Ofast -Os -s      \
    -march=native          \
    -Wall -Wextra -pedantic -Werror  -I Source/ -std=c++20