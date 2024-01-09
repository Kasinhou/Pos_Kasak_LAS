cmake_minimum_required(VERSION 3.18)
project(LAServer C)

set(CMAKE_C_STANDARD 11)

add_executable(LAServer main.c)
target_link_libraries(LAServer pthread)