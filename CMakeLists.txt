cmake_minimum_required(VERSION 3.0)
project(le VERSION 1.16.8)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

add_subdirectory(doc)
add_subdirectory(src)
add_subdirectory(misc)
