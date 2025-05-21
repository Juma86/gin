# build dependencies of main
cd ./builder-scripts/ ; ./buildmuclib.bash ; cd ..

# build main executable
cd ./builder-scripts/ ; ./buildmuc.bash    ; cd ..

echo "Project build attempt complete."
echo "        Find executable in ./binaries/"

echo "Preparing to install..."
sudo echo -n "" # do nothing.

sudo install ./binaries/muc       /usr/bin/muc
sudo install ./binaries/libmuc.so /usr/lib/libmuc.so

echo "installation finished."