# build dependencies of main
cd ./builder-scripts/ ; ./buildmuclib.bash ; cd ..

# build main executable
cd ./builder-scripts/ ; ./buildmuc.bash    ; cd ..

echo "Project build attempt complete."
echo "        Find executable in ./binaries/"