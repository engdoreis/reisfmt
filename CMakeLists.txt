cmake_minimum_required(VERSION 3.13)

set(NAME reisfmt)
project(${NAME} LANGUAGES C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


enable_testing()
add_subdirectory(tests EXCLUDE_FROM_ALL)

