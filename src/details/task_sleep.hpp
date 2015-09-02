#pragma once

#include <chrono>

namespace ell
{
  namespace details
  {
    /**
     * A dummy task that represents
     * the action of "sleeping".
     *
     * This is used to implement `ell::sleep()`.
     */
    class TaskSleep
    {
    public:
      template <typename Duration>
      explicit TaskSleep(TaskImplPtr /*parent_task*/, const Duration &duration)
      {
        auto now = std::chrono::system_clock::now();
        until_   = now + duration;
      }

      TaskSleep(const TaskSleep &o)
      {
        until_        = o.until_;
        wait_handler_ = o.wait_handler_;
      }

      const std::chrono::system_clock::time_point &until() const
      {
        return until_;
      }

      WaitHandler &wait_handler()
      {
        return wait_handler_;
      }

    private:
      std::chrono::system_clock::time_point until_;

      WaitHandler wait_handler_;
    };
  }
}