cmake_minimum_required(VERSION 3.0)
project(ell_tests)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -W -Wall -O2")

set(SOURCE_FILES main.cpp)
add_executable(test_basic ${SOURCE_FILES})

target_include_directories(test_basic PRIVATE ${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIR})
target_link_libraries(test_basic boost_coroutine boost_context boost_system)


add_executable(test_bench test_bench.cpp)

target_include_directories(test_bench PRIVATE ${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIR})
target_link_libraries(test_bench ${Boost_LIBRARIES})

