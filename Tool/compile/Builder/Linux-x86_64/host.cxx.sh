echo "Compiling: host.cxx"
g++ -c -o Temp/Object/host.cxx.o    \
    Source/aystl/host.cxx   \
    -O3 -Ofast -Os -s          \
    -march=native              \
    -Wall -Wextra -pedantic -Werror \
    -std=c++20 -I Source/