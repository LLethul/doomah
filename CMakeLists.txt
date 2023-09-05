cmake_minimum_required(VERSION 3.0)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

project(doomah)

include_directories(include)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O0")

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(output ${SOURCES})
# target_link_libraries (example ExampleLibrary)