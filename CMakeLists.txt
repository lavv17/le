cmake_minimum_required(VERSION 3.7)

execute_process(
    COMMAND gnulib/git-version-gen .tarball-version
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE git_version
)
string(REGEX REPLACE "-.*$" "" version ${git_version})

project(le VERSION ${version})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

add_subdirectory(doc)
add_subdirectory(src)
add_subdirectory(misc)
