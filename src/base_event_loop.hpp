#pragma once

#include "ell_fwd.hpp"

namespace ell
{
  /**
   * The publicly exposed EventLoop.
   *
   * This class defines the API available to the library's users.
   */
  template <typename EventLoopImpl>
  class BaseEventLoop
  {
  public:
    virtual ~BaseEventLoop() = default;

    /**
     * Arrange for a callback to be called as soon as possible.
     * The callback is called after call_soon() returns, when control returns to the
     * event loop.
     *
     * The callback will be wrapped in a task.
     */
    template <typename Callable>
    auto call_soon(const Callable &callable) -> TaskPtr<decltype(callable())>
    {
      return impl_.call_soon(callable);
    }

    /**
     * Run the event loop until the task `task` has been completed.
     */
    template <typename T>
    void run_until_complete(TaskPtr<T> task)
    {
      return impl_.run_until_complete(task);
    }

    template <typename T>
    void cancel_task(const TaskPtr<T> &task)
    {
      impl_.cancel_task(task);
    }

  private:
    EventLoopImpl impl_;
  };
}
