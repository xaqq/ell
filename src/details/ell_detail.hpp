#pragma once

namespace ell
{
  // We have one event loop per thread running at the same time.
  // We a run*() method is called, the event set itself as the current loop
  // for the thread. This allows static helper function to works as intended.
  namespace details
  {
    /**
     * Hides global thread_local variable inside a get/set functions
     */
    EventLoopImpl *set_current_event_loop(EventLoopImpl *loop, bool set = true)
    {
      static thread_local EventLoopImpl *current_loop = nullptr;
      if (set)
        current_loop = loop;
      return current_loop;
    }

    /**
     * Retrieve the current event loop for the current thread.
     */
    EventLoopImpl *get_current_event_loop()
    {
      return set_current_event_loop(nullptr, false);
    }
  }
}