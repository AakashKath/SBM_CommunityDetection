cmake_minimum_required(VERSION 3.10)

# Project name
project(SBM_CommunityDetection VERSION 0.1.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/lib)

# Collect all source files in the lib directory
file(GLOB LIB_SOURCES ${PROJECT_SOURCE_DIR}/lib/*.cpp)

# Find Graphviz package
find_package(PkgConfig REQUIRED)
pkg_check_modules(GRAPHVIZ REQUIRED libgvc libcdt)

# Include Graphviz directories
include_directories(${GRAPHVIZ_INCLUDE_DIRS})

# Add executable target with source files
add_executable(SBM_CommunityDetection main.cpp ${LIB_SOURCES})

# Link Graphviz libraries to your executable
target_link_libraries(SBM_CommunityDetection ${GRAPHVIZ_LIBRARIES})

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
