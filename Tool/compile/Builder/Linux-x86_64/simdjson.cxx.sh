echo "Compiling: simdjson.cxx"
g++ -c -o Temp/Object/simdjson.cxx.o    \
    Source/simdjson.cxx   \
    -O3 -Ofast -Os -s          \
    -march=native              \
    -Wall -Wextra -pedantic -Werror \
    -std=c++20 -I Source/