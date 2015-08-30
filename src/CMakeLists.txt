cmake_minimum_required(VERSION 3.0)
project(ell)
find_package(Boost REQUIRED COMPONENTS coroutine context system)

enable_testing()
add_subdirectory(tests)
