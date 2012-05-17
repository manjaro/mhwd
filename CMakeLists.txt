cmake_minimum_required (VERSION 2.6) 
project(mhwd)

SET(CMAKE_INSTALL_PREFIX "/usr/")

add_definitions(-Wall)

add_subdirectory(libmhwd)
add_subdirectory(mhwd)
add_subdirectory(scripts)
