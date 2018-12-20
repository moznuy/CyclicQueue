cmake_minimum_required(VERSION 3.13)
project(CyclicQueue)

set(CMAKE_CXX_STANDARD 14)

add_executable(CyclicQueue main.cpp Queue.cpp Queue.h)

target_link_libraries(CyclicQueue pthread)