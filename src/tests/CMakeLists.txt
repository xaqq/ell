cmake_minimum_required(VERSION 3.0)
project(ell_tests)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -W -Wall")

set(SOURCE_FILES main.cpp)
add_executable(test_basic ${SOURCE_FILES})

target_include_directories(test_basic PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_basic boost_coroutine boost_context boost_system)
