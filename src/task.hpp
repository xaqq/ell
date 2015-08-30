#pragma once

#include "details/task_impl.hpp"
#include "ell_fwd.hpp"

namespace ell
{
  /**
   * A task is responsible for executing coroutine object in an event loop.
   *
   * The Task class, which is part of the public API is templated
   * on the return type of the task.
   *
   * This type doesn't do type erasure in order to provide compile time
   * type-safety to the library user.
   *
   * A Task object is not copyable.
   *
   * @note The type-erased type that is used internally is details::TaskImpl
   * @note Due to implementation, no matter `<T>` the size of the object is the same.
   */
  template <typename ReturnType>
  class Task
  {
    Task(const Task &) = delete;
    Task(Task &&) = delete;

    Task &operator=(const Task &) = delete;
    Task &operator=(Task &&) = delete;

    template <typename Callable>
    Task(const Callable &callable)
        : impl_(callable)
    {
    }

  public:
    ReturnType get_result()
    {
      return impl_.get_result<ReturnType>();
    }

  private:
    details::TaskImpl impl_;

    friend EventLoop;
    friend details::TaskBuilder;
  };
}
