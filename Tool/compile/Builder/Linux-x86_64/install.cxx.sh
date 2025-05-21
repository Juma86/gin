echo "Compiling: install.cxx"
g++ -c -o Temp/Object/install.cxx.o \
    Source/install.cxx         \
    -O3 -Ofast -Os -s          \
    -march=native              \
    -Wall -Wextra -pedantic -no-pie -Werror -I Source/