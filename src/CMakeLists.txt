cmake_minimum_required(VERSION 3.0)
project(ell)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -W -Wall -O3")

find_package(Boost REQUIRED COMPONENTS coroutine context system)


set(SOURCE_FILES task.cpp)
add_library(ell ${SOURCE_FILES})

target_include_directories(ell PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIR})
target_link_libraries(ell ${Boost_LIBRARIES})

add_subdirectory(tests)