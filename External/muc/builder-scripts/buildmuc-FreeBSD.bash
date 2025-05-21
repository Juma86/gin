echo "Building main executable ..."

# build main object
g++15 -c -Wall -Wextra -Werror -O3 -Os -s -Ofast \
    -o ./../objects/main.o ./../source/main.cxx

echo "Linking ..."

# build main executable
g++15 -L./../binaries/ -Wl,-rpath,"\$ORIGIN" \
    -o ./../binaries/muc ./../objects/main.o -lmuc-V1
