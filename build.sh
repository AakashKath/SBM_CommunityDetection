#!/bin/bash

# TODO: Need to install graphviz-dev via script

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to the build directory
cd build

# Run cmake and make
cmake ..
make

# Navigate back to the project root
cd ..

# Notify the user where the executable is located
echo "Executable created in bin directory."

# TODO: Maybe run the executable
# ./build/bin/SBM_CommunityDetection
# rm -r build
