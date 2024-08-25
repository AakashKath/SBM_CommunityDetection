cmake_minimum_required(VERSION 3.10)

# Project name
project(SBM_CommunityDetection VERSION 0.1.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Define library name
set(LIBRARY_NAME
    SBM_CommunityDetection
)

# Collect all source files in the src and lib directories
file(GLOB SRC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp)

# Find Graphviz package
find_package(PkgConfig REQUIRED)
pkg_check_modules(GRAPHVIZ REQUIRED libgvc libcdt)

# Include Graphviz directories
include_directories(${GRAPHVIZ_INCLUDE_DIRS})

# Add executable target with source files
add_executable(${LIBRARY_NAME} src/main.cpp ${SRC_SOURCES} ${LIB_SOURCES})

# Link Graphviz libraries to your executable
target_link_libraries(${LIBRARY_NAME} ${GRAPHVIZ_LIBRARIES})

# Include directories for the library
target_include_directories(${LIBRARY_NAME} PRIVATE
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/DCD>
$<INSTALL_INTERFACE:/include/DCD>
)
target_include_directories(${LIBRARY_NAME} PRIVATE
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/StreamBP>
$<INSTALL_INTERFACE:/include/StreamBP>
)
target_include_directories(${LIBRARY_NAME} PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
$<INSTALL_INTERFACE:include>
)

# Include root project path
target_include_directories(${LIBRARY_NAME} PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
$<INSTALL_INTERFACE:>
)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
