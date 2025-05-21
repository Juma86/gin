echo "Compiling: loadginconfig.cxx"
g++ -c -o Temp/Object/loadginconfig.cxx.o \
    Source/main/loadginconfig.cxx         \
    -O3 -Ofast -Os -s          \
    -march=native              \
    -Wall -Wextra -pedantic -Werror -I Source/