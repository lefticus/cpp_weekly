#!/bin/bash

# Bash script to compile each c++ file into its own executable

# Loop over each file in the directory
for file in *.cpp
do
   # Compile file into an executable, using output filename as the cpp filename
   g++ $file -o ${file%.cpp} -O3 -march=native -std=c++23
done
