#!/bin/bash

# TODO: Need to install graphviz-dev via script

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to the build directory
cd build

# Run cmake and make
cmake .. -DCMAKE_BUILD_TYPE=Debug # -DUSE_SANITIZER=Address # Used for memory leakage detection
make

# Navigate back to the project root
cd ..

# Notify the user where the executable is located
echo "Executable created in build/bin directory."

# To run the program use below command
# ./build/bin/SBM_CommunityDetection
# rm -r build
