cmake_minimum_required(VERSION 3.30.2)

# Project name
project(SBM_CommunityDetection VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define library name
set(LIBRARY_NAME SBM_CommunityDetection)

# Define test name
set(TEST_NAME SBM_Test)

# Define a preprocessor definition with the PNG output directory
add_compile_definitions(TEST_OUTPUT_DIRECTORY="${TEST_OUTPUT_DIRECTORY}")
add_compile_definitions(TEST_DATA_DIRECTORY="${TEST_DATA_DIRECTORY}")
add_compile_definitions(CONFIG_DIRECTORY="${CONFIG_DIRECTORY}")

# Collect all source files in the src and lib directories
file(GLOB SRC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM SRC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
file(GLOB UTILS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp)
file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
file(GLOB SCRIPTS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.cpp)

# Combine all source files into one list
set(COMMON_SOURCES
    ${SRC_SOURCES}
    ${UTILS_SOURCES}
    ${SCRIPTS_SOURCES}
)

# Find Graphviz package
find_package(PkgConfig REQUIRED)
pkg_check_modules(GRAPHVIZ REQUIRED libgvc libcdt)

# Include Graphviz directories
include_directories(${GRAPHVIZ_INCLUDE_DIRS})

include(FetchContent)

# Add external libraries
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)

FetchContent_Declare(
    ortools
    GIT_REPOSITORY https://github.com/google/or-tools.git
    GIT_TAG v9.11
)

# Enable fetching dependencies for OR-Tools
set(BUILD_DEPS ON CACHE BOOL "Fetch and build all dependencies for OR-Tools")

FetchContent_MakeAvailable(json googletest ortools)

# Add executable target with source files
add_executable(${LIBRARY_NAME} src/main.cpp ${COMMON_SOURCES})
add_executable(${TEST_NAME} ${TEST_SOURCES} ${COMMON_SOURCES})

# Link Graphviz libraries to your executable
target_link_libraries(${LIBRARY_NAME} nlohmann_json ortools::ortools ${GRAPHVIZ_LIBRARIES})
target_link_libraries(${TEST_NAME} nlohmann_json ortools::ortools gtest gtest_main ${GRAPHVIZ_LIBRARIES})

# Include directories for the library
set(COMMON_INCLUDED_DIRECTORIES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/DCD>
    $<INSTALL_INTERFACE:/include/DCD>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/ACD>
    $<INSTALL_INTERFACE:/include/ACD>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/StreamBP>
    $<INSTALL_INTERFACE:/include/StreamBP>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/IP_Solver>
    $<INSTALL_INTERFACE:/include/IP_Solver>
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Include common libraries listed above
target_include_directories(${LIBRARY_NAME} PRIVATE ${COMMON_INCLUDED_DIRECTORIES})
target_include_directories(${TEST_NAME} PRIVATE ${COMMON_INCLUDED_DIRECTORIES})

# Include public facing libraries
target_include_directories(${LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Set up testing
enable_testing()
include(GoogleTest)
gtest_discover_tests(${TEST_NAME})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION "SBM Community Detection Project")
set(CPACK_PACKAGE_CONTACT "aakashkath2@gmail.com")
include(CPack)
