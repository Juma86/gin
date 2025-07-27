# build dependencies of main

set -e

mkdir -p ./objects/ ./binaries/

cd ./builder-scripts/ ; bash ./buildmuclib.bash ; cd ..

# build library
sudo install ./binaries/libmuc.so /usr/lib/libmuc.so
sudo ldconfig

# build main executable
cd ./builder-scripts/ ; bash ./buildmuc.bash    ; cd ..

echo "Project build attempt complete."
echo "        Find executable in ./binaries/"

echo "Preparing to install..."
sudo echo -n "" # do nothing.

sudo install ./binaries/muc       /usr/bin/muc

sudo ldconfig

echo "installation finished."