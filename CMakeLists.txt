cmake_minimum_required(VERSION 2.6)
project(suffixtree)

SET(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")

add_library(suffixtree STATIC suffixtree.cpp)

add_executable(substrings main.cpp)
target_link_libraries(substrings suffixtree)
