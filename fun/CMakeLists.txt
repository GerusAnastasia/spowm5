cmake_minimum_required(VERSION 3.16)
project(fun CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
add_library(fun SHARED library.cpp library.h)

target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_link_libraries(fun rt)
