cmake_minimum_required(VERSION 3.13)

set(NAME reisfmt)

project(${NAME} LANGUAGES C CXX ASM)

# For lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic)

add_subdirectory(tests)

