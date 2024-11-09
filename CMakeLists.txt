cmake_minimum_required(VERSION 3.29)
project(PLC_INTERPRETER)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(PLC_INTERPRETER
    klang.cpp
)


