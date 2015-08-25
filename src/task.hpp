#pragma once

#include "details/task_impl.hpp"
#include "ell_fwd.hpp"
#include "ell.hpp"

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
   */
  template <typename ReturnType>
  class Task
  {
    Task(const Task &) = delete;
    Task() = default;


    /**
     * Create a TaskPtr<T> for use by the end-user.
     *
     * The Task<T> object is linked to the TaskImpl object `impl_ptr`.
     */
    static TaskPtr<ReturnType> create(const details::TaskImplPtr &impl_ptr)
    {
      TaskPtr<ReturnType> ptr(new Task<ReturnType>());
      ptr->impl_ = impl_ptr;

      return ptr;
    }

  public:
    ReturnType get_result()
    {
      return impl_->get_result<ReturnType>();
    }

  private:
    details::TaskImplPtr impl_;

    friend EventLoop;
  };
}
