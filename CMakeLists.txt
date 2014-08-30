cmake_minimum_required(VERSION 2.8)

project(ward CXX)

# Main library

set(WARD_SOURCES
    src/Ward.h
    src/Ward.cpp
)

if(WIN32)
    LIST(APPEND WARD_SOURCES src/Win32.cpp)
else()
    LIST(APPEND WARD_SOURCES src/POSIX.cpp)
    add_definitions(-D_DEBUG=1)
endif()

add_library(ward
    ${WARD_SOURCES}
)

# Test app

include_directories(src)
add_executable(wardtest
    test/wardtest.cpp
)
target_link_libraries(wardtest
    ward
)
