cmake_minimum_required(VERSION 2.8)

project(ward CXX)

# Main library

add_library(ward
    src/ward.h
    src/ward.cpp
)

# Test app

include_directories(src)
add_executable(wardtest
    test/wardtest.cpp
)
target_link_libraries(wardtest
    ward
)
