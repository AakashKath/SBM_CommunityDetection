cmake_minimum_required(VERSION 3.10)

# Project name
project(SBM_CommunityDetection VERSION 0.1.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define library name
set(LIBRARY_NAME
    SBM_CommunityDetection
)

# Define test name
set(TEST_NAME
    SBM_Test
)

# List all submodules
set(SUBMODULES
    "json"
    "googletest"
)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

foreach(SUBMODULE ${SUBMODULES})
    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/lib/${SUBMODULE}/CMakeLists.txt")
        message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
    endif()
endforeach()

# Define a preprocessor definition with the PNG output directory
add_compile_definitions(TEST_OUTPUT_DIRECTORY="${TEST_OUTPUT_DIRECTORY}")

# Collect all source files in the src and lib directories
file(GLOB SRC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM SRC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
file(GLOB LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp)
file(GLOB UTILS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp)
file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)

# Combine all source files into one list
set(COMMON_SOURCES
    ${SRC_SOURCES}
    ${LIB_SOURCES}
    ${UTILS_SOURCES}
)

# Find Graphviz package
find_package(PkgConfig REQUIRED)
pkg_check_modules(GRAPHVIZ REQUIRED libgvc libcdt)

# Include Graphviz directories
include_directories(${GRAPHVIZ_INCLUDE_DIRS})

# Add external libraries
add_subdirectory(lib/json)
add_subdirectory(lib/googletest)

# Add executable target with source files
add_executable(${LIBRARY_NAME} src/main.cpp ${COMMON_SOURCES})
add_executable(${TEST_NAME} ${TEST_SOURCES} ${COMMON_SOURCES})

# Link Graphviz libraries to your executable
target_link_libraries(${LIBRARY_NAME} ${GRAPHVIZ_LIBRARIES})
target_link_libraries(${TEST_NAME} gtest gtest_main ${GRAPHVIZ_LIBRARIES})

# Include directories for the library
set(COMMON_INCLUDED_DIRECTORIES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/DCD>
    $<INSTALL_INTERFACE:/include/DCD>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/StreamBP>
    $<INSTALL_INTERFACE:/include/StreamBP>
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/json/single_include
)

# Include common libraries listed above
target_include_directories(${LIBRARY_NAME} PRIVATE ${COMMON_INCLUDED_DIRECTORIES})
target_include_directories(${TEST_NAME} PRIVATE ${COMMON_INCLUDED_DIRECTORIES})

# Include public facing libraries
target_include_directories(${LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Include googletest library
target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/googletest/googletest/include)

# Set up testing
enable_testing()
include(GoogleTest)
gtest_discover_tests(${TEST_NAME})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION "SBM Community Detection Project")
set(CPACK_PACKAGE_CONTACT "aakashkath2@gmail.com")
include(CPack)
