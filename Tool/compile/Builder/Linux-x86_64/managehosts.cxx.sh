echo "Compiling: managehosts.cxx"
g++ -c -o Temp/Object/managehosts.cxx.o \
    Source/managehosts.cxx         \
    -O3 -Ofast -Os -s              \
    -march=native                  \
    -Wall -Wextra -pedantic -Werror -I Source/