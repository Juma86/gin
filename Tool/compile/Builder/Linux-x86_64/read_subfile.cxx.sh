echo "Compiling: read_subfile.cxx"
g++ -c -o Temp/Object/read_subfile.cxx.o    \
    Source/install/muc/read_subfile.cxx   \
    -O3 -Ofast -Os -s          \
    -march=native              \
    -Wall -Wextra -pedantic -Werror \
    -std=c++20 -I Source/