cmake_minimum_required(VERSION 3.12)

project(bdb_rebuild)

set (CMAKE_CXX_STANDARD 17)

include_directories(submodules/json/src)
include_directories(include)

add_executable(bdb_rebuild src/main.cpp)
