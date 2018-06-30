cmake_minimum_required(VERSION 2.8)

project(persistent_set)

include_directories(${HUFFMAN_SOURCE_DIR})

add_library(persistent_set_library STATIC persistent_set.cpp persistent_set.h)

add_executable(persistent_set_testing
        persistent_set_testing/counted.cpp
        persistent_set_testing/counted.h
        persistent_set_testing/fault_injection.cpp
        persistent_set_testing/fault_injection.h
        persistent_set_testing/persistent_set_testing.cpp
        persistent_set_testing/gtest/gtest-all.cc
        persistent_set_testing/gtest/gtest.h
        persistent_set_testing/gtest/gtest_main.cc)

add_executable(testing my_example/test.cpp)


if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined,leak -D_GLIBCXX_DEBUG")
endif ()

target_link_libraries(persistent_set_testing persistent_set_library -lpthread)
target_link_libraries(testing persistent_set_library -lpthread)