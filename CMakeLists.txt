cmake_minimum_required(VERSION 3.6)
project(echo_server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp error_check.cpp unp.h)
add_executable(echo_server ${SOURCE_FILES})