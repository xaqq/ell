#pragma once

#include "ell_fwd.hpp"

namespace ell
{
  namespace details
  {
    class DefaultEventLoop;
  }
  using EventLoop = details::DefaultEventLoop;

  // We have one event loop per thread running at the same time.
  // We a run*() method is called, the event set itself as the current loop
  // for the thread. This allows static helper function to works as intended.
  namespace details
  {
    /**
     * Hides global thread_local variable inside a get/set functions
     */
    EventLoop *set_current_event_loop(EventLoop *loop, bool set = true)
    {
      static thread_local EventLoop *current_loop = nullptr;
      if (set)
        current_loop = loop;
      return current_loop;
    }

    /**
     * Retrieve the current event loop for the current thread.
     */
    EventLoop *get_current_event_loop()
    {
      return set_current_event_loop(nullptr, false);
    }
  }
}

#include "task.hpp"
#include "details/default_event_loop.hpp"

namespace ell
{

  /**
  * Simply yield, being nice and giving other a chance to run!
  */
  void yield()
  {
    details::get_current_event_loop()->suspend_current_task();
  }

  /**
   * Yield to an other callable, waiting for this callable
   * to return in order to continue.
   */
  template <typename Callable>
  auto yield(const Callable &callable) -> decltype(callable())
  {
    auto loop = details::get_current_event_loop();
    return loop->yield(callable);
  }
}