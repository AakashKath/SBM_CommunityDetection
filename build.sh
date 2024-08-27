#!/bin/bash

# Function to check if a package is installed and install it if necessary
check_and_install() {
    local PKG_NAME=$1
    local MANUAL_PATHS=("/usr/local/bin" "/opt/bin")

    if [ "$PKG_NAME" == "cmake" ]; then
        # Special handling for cmake
        if command -v cmake > /dev/null 2>&1; then
            echo "cmake is already installed."
        else
            echo "cmake is not installed. Installing manually..."
            wget https://github.com/Kitware/CMake/releases/download/v3.30.2/cmake-3.30.2.tar.gz
            tar -xzvf cmake-3.30.2.tar.gz
            cd cmake-3.30.2
            ./bootstrap
            make
            sudo make install
            cd ..
            echo "cmake has been installed from source."
        fi
    else
        # Check if the package is installed via package manager
        if dpkg -l | grep -q "^ii  $PKG_NAME "; then
            echo "$PKG_NAME is already installed via package manager."
        else
            # Check if the package is manually installed (by searching common paths)
            for path in "${MANUAL_PATHS[@]}"; do
                if [ -x "$path/$PKG_NAME" ]; then
                    echo "$PKG_NAME is manually installed and available in $path."
                    return
                fi
            done

            # Check if the development package files exist
            if dpkg -L "$PKG_NAME" > /dev/null 2>&1; then
                echo "$PKG_NAME is installed (development files found)."
                return
            fi

            # Package is not installed, proceed to install
            echo "$PKG_NAME is not installed. Installing..."
            sudo apt-get install -y "$PKG_NAME"
        fi
    fi
}

# Update package list
sudo apt-get update -y

# Check and install necessary packages
check_and_install wget
check_and_install libssl-dev
check_and_install build-essential
check_and_install pkg-config
check_and_install libgraphviz-dev
check_and_install cmake

# Initialize and update git submodules
git submodule update --init --recursive

# Create build directory if it doesn't exist
mkdir -p build

# Run cmake to configure the project
cmake . # -DCMAKE_BUILD_TYPE=Debug -DUSE_SANITIZER=Address # Used for memory leakage detection

# Build the project
make

# Run the tests using CTest
ctest --output-on-failure

# Navigate back to the project root
cd ..

# Notify the user where the executable is located
echo "Executable created in build/bin directory."

# To run the program, use the command below
# ./build/bin/SBM_CommunityDetection

# Uncomment to clean up build directory
# rm -r build
