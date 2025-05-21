echo "Compiling: test_aystl_algorithm.cxx"
g++ -c -o Temp/Object/test_aystl_algorithm.cxx.o \
    Source/tests/aystl/algorithm/test_aystl_algorithm.cxx         \
    -O3 -Ofast -Os -s          \
    -march=native              \
    -Wall -Wextra -pedantic -Werror -I Source/