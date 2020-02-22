cmake_minimum_required(VERSION 3.5)

project(rt_periodic_thread LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(periodic_thread periodic_thread.cpp)
target_link_libraries(periodic_thread pthread rt)
