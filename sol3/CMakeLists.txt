cmake_minimum_required(VERSION 3.27)
project(sol2i)

set(CMAKE_CXX_STANDARD 23)

add_executable(sol2i main.cpp
                kdtree.hpp
        SafePriorityQueue.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fopenmp")