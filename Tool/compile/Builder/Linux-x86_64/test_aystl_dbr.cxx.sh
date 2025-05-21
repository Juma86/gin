echo "Compiling: test_aystl_dbr.cxx"
g++ -c -o Temp/Object/test_aystl_dbr.cxx.o \
    Source/tests/aystl/dbr/test_aystl_dbr.cxx         \
    -O3 -Ofast -Os -s          \
    -march=native              \
    -Wall -Wextra -pedantic -Werror -I Source/