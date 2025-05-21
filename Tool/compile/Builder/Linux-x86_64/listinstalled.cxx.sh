echo "Compiling: listinstalled.cxx"
g++ -c -o Temp/Object/listinstalled.cxx.o \
    Source/listinstalled.cxx         \
    -O3 -Ofast -Os -s          \
    -march=native              \
    -Wall -Wextra -pedantic -Werror -I Source/