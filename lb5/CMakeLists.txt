cmake_minimum_required(VERSION 3.16)
project(lb5 CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
add_executable(lb5 main.cpp laba_linux.h laba_linux.h)
target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_link_libraries(lb5 rt)
target_link_libraries(lb5 dl)