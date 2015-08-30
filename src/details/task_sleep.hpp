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
      explicit TaskSleep(TaskImplPtr parent_task, const Duration &duration)
          : parent_(parent_task)
      {
        auto now = std::chrono::system_clock::now();
        until_   = now + duration;
      }

      const std::chrono::system_clock::time_point &until() const
      {
        return until_;
      }

      const TaskImplPtr &parent() const
      {
        return parent_;
      }

    private:
      std::chrono::system_clock::time_point until_;

      /**
       * The task that initiated the sleep.
       */
      TaskImplPtr parent_;
    };
  }
}