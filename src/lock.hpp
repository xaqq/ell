#pragma once

#include <queue>
#include <vector>
#include "ell_fwd.hpp"
#include "ell.hpp"
#include "details/task_impl.hpp"
#include "details/wait_handler.hpp"
#include "details/ell_assert.hpp"

namespace ell
{
  /**
   * Primitive lock objects.
   *
   * A primitive lock is a synchronization primitive that is not owned by a particular
   * coroutine when locked.
   * A primitive lock is in one of two states, ‘locked’ or ‘unlocked’.
   * It is created in the unlocked state. It has two basic methods, lock() and unlock().
   * When the state is unlocked, lock() changes the state to locked and returns
   * immediately.
   * When the state is locked, lock() blocks until a call to unlock() in another coroutine
   * changes it to unlocked,
   * then the lock() call resets it to locked and returns.
   * The unlock() method should only be called in the locked state; it changes the state
   * to unlocked
   * and returns immediately. If an attempt is made to release an unlocked lock, a
   * RuntimeError will be raised.
   *
   * @warning This class is not thread-safe.
   */
  class Lock
  {
  public:
    Lock()
        : locked_(false)
    {
    }
    void lock()
    {
      auto loop = details::get_current_event_loop();
      ELL_ASSERT(loop, "No event loop.");

      if (!locked_)
      {
        locked_ = true;
        return;
      }

      while (locked_)
      {
        loop->attach_wait_handler(wait_, loop->current_task());
        loop->current_task_suspend();
        // someone may wake up before us.
      }
      locked_ = true;
    }

    void unlock()
    {
      auto loop = details::get_current_event_loop();
      ELL_ASSERT(loop, "No event loop.");
      loop->detach_wait_handler(wait_);
    }

  private:
    details::WaitHandler wait_;
    bool locked_;
    std::vector<details::TaskImplPtr> waiters_;
  };
}
