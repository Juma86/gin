echo "Compiling: test_aystl_fs.cxx"
g++ -c -o Temp/Object/test_aystl_fs.cxx.o \
    Source/tests/aystl/fs/test_aystl_fs.cxx         \
    -O3 -Ofast -Os -s          \
    -march=native              \
    -Wall -Wextra -pedantic -Werror -I Source/