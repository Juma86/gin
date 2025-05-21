echo "Compiling: uninstall.cxx"
g++ -c -o Temp/Object/uninstall.cxx.o \
    Source/uninstall.cxx         \
    -O3 -Ofast -Os -s          \
    -march=native              \
    -Wall -Wextra -pedantic -no-pie -Werror -I Source/