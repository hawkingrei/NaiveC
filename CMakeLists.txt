cmake_minimum_required(VERSION 3.0)
project(NaiveC)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

aux_source_directory(src SOURCE_FILES)

add_executable(naive_c ${SOURCE_FILES})
