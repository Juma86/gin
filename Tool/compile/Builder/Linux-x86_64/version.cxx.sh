echo "Compiling: version.cxx"
g++ -c -o Temp/Object/version.cxx.o \
    Source/version.cxx         \
    -O3 -Ofast -Os -s          \
    -march=native              \
    -Wall -Wextra -pedantic -Werror -I Source/