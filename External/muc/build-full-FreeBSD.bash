# build dependencies of main
cd ./builder-scripts/ ; ./buildmuclib-FreeBSD.bash ; cd ..

# build main executable
cd ./builder-scripts/ ; ./buildmuc-FreeBSD.bash    ; cd ..

echo "Project build attempt complete."
echo "        Find executable in ./binaries/"
