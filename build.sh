#!/bin/bash

# Function to check if a package is installed
check_and_install() {
    PKG_NAME=$1
    if ! dpkg -l | grep -q "^ii  $PKG_NAME "; then
        echo "$PKG_NAME is not installed. Installing..."
        sudo apt install -y $PKG_NAME
    else
        echo "$PKG_NAME is already installed."
    fi
}

sudo apt update

# Check and install necessary packages
check_and_install cmake
check_and_install build-essential
check_and_install pkg-config
check_and_install libgraphviz-dev


# Initialize and update git submodules
git submodule update --init --recursive

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to the build directory
cd build

# Run cmake to configure the project with Debug build type
cmake .. -DCMAKE_BUILD_TYPE=Debug # -DUSE_SANITIZER=Address # Used for memory leakage detection

# Build the project
make

# Run the tests using CTest
ctest --output-on-failure

# Navigate back to the project root
cd ..

# Notify the user where the executable is located
echo "Executable created in build/bin directory."

# To run the program use the command below
# ./build/bin/SBM_CommunityDetection

# Uncomment to clean up build directory
# rm -r build
