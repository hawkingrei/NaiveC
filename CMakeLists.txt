cmake_minimum_required(VERSION 3.0)
project(NaiveC)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

aux_source_directory(src SOURCE_FILES)

add_executable(naive_c ${SOURCE_FILES})

llvm_map_components_to_libnames(llvm_libs support core irreader bitwriter)

target_link_libraries(naive_c ${llvm_libs})
