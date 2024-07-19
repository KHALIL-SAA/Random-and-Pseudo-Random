cmake_minimum_required(VERSION 3.25)
project(ProjetSysteme C)

set(CMAKE_C_STANDARD 11)

add_executable(ProjetSysteme main.c)
add_link_libraries(ProjetSysteme m pthread)
