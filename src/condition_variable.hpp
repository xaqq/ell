#pragma once

#include <queue>
#include <vector>
#include "ell_fwd.hpp"
#include "ell.hpp"
#include "details/task_impl.hpp"

namespace ell
{
  /**
   * A single thread condition variable.
   *
   * For use between coroutine on the same thread.
   */
  class ConditionVariable
  {
  public:
    void wait()
    {
      auto loop = details::get_current_event_loop();
      assert(loop);

      waiters_.push_back(loop->current_task());
      loop->mark_inactive(loop->current_task());
      loop->suspend_current_task();

      // could built in predicate check too.
    }

    void notify_all()
    {
      auto loop = details::get_current_event_loop();
      assert(loop);

      for (const auto &waiter : waiters_)
      {
        loop->mark_active(waiter);
      }
      waiters_.clear();
    }

    bool status;
    std::vector<details::TaskImplPtr> waiters_;
  };
}