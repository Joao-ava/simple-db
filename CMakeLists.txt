cmake_minimum_required(VERSION 4.1)
project(simple_db C)

set(CMAKE_C_STANDARD 11)

add_executable(simple_db src/main.c
        src/repl.c
        src/repl.h)
