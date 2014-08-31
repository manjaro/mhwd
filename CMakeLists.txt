cmake_minimum_required (VERSION 2.6) 
project(mhwd)

SET(CMAKE_INSTALL_PREFIX "/usr/")
SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER clang++)

add_definitions(-std=c++11 -O0 -g -Wall -Wextra -Wunreachable-code -Werror)

add_subdirectory(libmhwd)
add_subdirectory(src)
add_subdirectory(scripts)
