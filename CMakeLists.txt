cmake_minimum_required(VERSION 4.0)
project(My_Malloc)

set(CMAKE_CXX_STANDARD 20)

add_executable(My_Malloc main.c
        defs.h
        functions.c
        main.c
        func.c)
