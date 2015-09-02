#pragma once

#include <queue>
#include <vector>
#include "ell_fwd.hpp"
#include "ell.hpp"
#include "details/task_impl.hpp"
#include "lock.hpp"

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

      std::cout << "LOL " << wait_.id() << std::endl;
      //      lock_.lock();
      loop->attach_wait_handler(wait_, loop->current_task());
      loop->suspend_current_task();

      // could built in predicate check too.
    }

    void notify_all()
    {
      auto loop = details::get_current_event_loop();
      assert(loop);

      loop->detach_wait_handler(wait_);
      wait_.tasks_.clear();
      //      lock_.unlock();
    }

    details::WaitHandler wait_;
    // Lock lock_;
  };
}