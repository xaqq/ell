#pragma once

#include "ell_fwd.hpp"

namespace ell
{
  /**
   * The base interface for an event loop.
   *
   * This class uses the CRTP pattern for compile time polymorphism.
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
      return static_cast<EventLoopImpl *>(this)->call_soon_impl(callable);
    }

    /**
     * Run the event loop until the task `task` has been completed.
     */
    template <typename T>
    void run_until_complete(TaskPtr<T> task)
    {
      return static_cast<EventLoopImpl *>(this)->run_until_complete_impl(task);
    }

    /**
     * Suspend the current task.
     *
     * The current task will be rescheduled during the next iteration
     * of the event loop.
     */
    void suspend_current_task()
    {
      static_cast<EventLoopImpl *>(this)->suspend_current_task_impl();
    }

    /**
     * Make the current task go to sleep for a certain duration.
     */
    template <typename Duration>
    void sleep_current_task(const Duration &duration)
    {
      static_cast<EventLoopImpl *>(this)->sleep_current_task_impl(duration);
    }

    /**
     * Yield to another callable
     */
    template <typename Callable>
    auto yield(const Callable &callable) -> decltype(callable())
    {
      return static_cast<EventLoopImpl *>(this)->yield_impl(callable);
    }
  };
}
