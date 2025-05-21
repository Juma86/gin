#!/bin/bash

echo "Building main executable ..."

# build main object
g++ -c -Wall -Wextra -Werror -O3 -Os -s -Ofast  \
    -o ./../objects/main.o ./../source/main.cxx \
    -DHELP_MENU_B64="\"$(cat ./../source/help-menu.txt | base64 --wrap=0)\""

# build main executable
g++ -L./../Binaries/ -Wl,-rpath,"\$ORIGIN" \
    -o ./../binaries/muc ./../objects/main.o -lmuc
