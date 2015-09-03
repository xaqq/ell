cmake_minimum_required(VERSION 3.0)
project(ell)
find_package(Boost REQUIRED COMPONENTS coroutine context system)

enable_testing()
add_subdirectory(tests)

set(ELL_HEADERS
base_event_loop.hpp
condition_variable.hpp
ell.hpp
ell_fwd.hpp
lock.hpp
queue.hpp
task.hpp
)

set(ELL_HEADER_DETAILS
details/default_event_loop.hpp
details/ell_assert.hpp
details/ell_log.hpp
details/result_holder.hpp
details/task_builder.hpp
details/task_impl.hpp
details/task_sleep.hpp
details/valgrind_allocator.hpp
details/wait_handler.hpp
)
