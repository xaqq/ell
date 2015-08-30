cmake_minimum_required(VERSION 3.0)
project(ell)
find_package(Boost REQUIRED COMPONENTS coroutine context system)
add_subdirectory(tests)
