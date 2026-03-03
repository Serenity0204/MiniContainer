cmake_minimum_required(VERSION 3.16)

# project definition
project(mini_container LANGUAGES CXX) 

# set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compile options
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
)

# include directories
include_directories(${PROJECT_SOURCE_DIR}/includes)

# src files
file(GLOB SRC_FILES
    ${PROJECT_SOURCE_DIR}/srcs/*.cpp
)

# generate executable
add_executable(mini-container ${SRC_FILES})

# Linux specific macros
target_compile_definitions(mini-container PRIVATE _GNU_SOURCE)

