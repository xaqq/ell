cmake_minimum_required(VERSION 3.0)
project(ell_tests)

set(ELL_ENABLE_TRACE 1)
set(ELL_ENABLE_DEBUG 1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -W -Wall -O0 -g")

if (ELL_ENABLE_DEBUG)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPDLOG_DEBUG_ON=1 -DELL_ENABLE_DEBUG=1")
endif()
if (ELL_ENABLE_TRACE)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPDLOG_TRACE_ON=1 -DELL_ENABLE_TRACE=1")
endif()

set(SOURCE_FILES main.cpp)
add_executable(test_basic ${SOURCE_FILES})

target_include_directories(test_basic PRIVATE ${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../spdlog/include/
)
target_link_libraries(test_basic boost_coroutine boost_context boost_system pthread)


add_executable(test_bench test_bench.cpp)
target_include_directories(test_bench PRIVATE ${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../spdlog/include/
)
target_link_libraries(test_bench ${Boost_LIBRARIES} pthread)

find_package(GTest REQUIRED)

function(create_test file)
   get_filename_component(test_name ${file} NAME_WE)
   add_executable(${test_name} ${file})

   target_include_directories(${test_name} PRIVATE
           ${GTEST_INCLUDE_DIRS}
           ${CMAKE_CURRENT_SOURCE_DIR}/../
           ${Boost_INCLUDE_DIR}
           ${CMAKE_CURRENT_SOURCE_DIR}/../spdlog/include/
   )

   target_link_libraries(${test_name} ${GTEST_BOTH_LIBRARIES} ${Boost_LIBRARIES} pthread)
endfunction()

function(add_gtest_test file)
    get_filename_component(test_name ${file} NAME_WE)
    create_test(${file})
    add_test(${test_name} ${test_name})
endfunction()

add_gtest_test(test_result_holder.cpp)
add_gtest_test(test_task_builder.cpp)
add_gtest_test(test_sleep.cpp)
add_gtest_test(test_queue.cpp)
add_gtest_test(test_condition_variable.cpp)
# ctest cannot run this test correctly.
create_test(test_signal_handler.cpp)
