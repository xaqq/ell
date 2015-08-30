#pragma once

#include "ell.hpp"
#include "base_event_loop.hpp"
#include "details/task_builder.hpp"
#include <iostream>

namespace ell
{
  namespace details
  {
    class DefaultEventLoop : public BaseEventLoop<DefaultEventLoop>
    {
    private:
      template <typename Callable>
      auto call_soon_impl(const Callable &callable) -> TaskPtr<decltype(callable())>
      {
        using ReturnType         = decltype(callable());
        TaskPtr<ReturnType> task = builder_.build(callable);
        TaskImplPtr impl(task, &task->impl_);

        new_tasks_.push_back(impl);
        return task;
      }

      template <typename T>
      void run_until_complete_impl(TaskPtr<T> task)
      {
        set_current_event_loop(this);

        while (!task->impl_.is_complete())
        {
          schedule();
        }
      }

      void suspend_current_task_impl()
      {
        current_task_->suspend();
      }

      /**
       * Run the scheduler, effectively giving CPU time to some coroutine.
       */
      void schedule()
      {
        move_tasks();

        for (auto &task : active_tasks_)
        {
          current_task_ = task;
          task->resume();

          if (task->is_complete())
          {
            // std::cout << "Task complete !" << std::endl;
            completed_tasks_.push_back(task);
            for (auto &child_task : task->dependants_)
              mark_active(child_task);
          }
          else
          {
            //  std::cout << "Task NOT complete" << std::endl;
          }
        }
      }

      template <typename Callable>
      auto yield_impl(const Callable &callable) -> decltype(callable())
      {
        auto task = call_soon(callable);

        task->impl_.dependants_.push_back(current_task_);
        new_inactive_tasks_.push_back(current_task_);
        current_task_->suspend();
        return task->get_result();
      }

      /**
       * Move the newly created task into the
       * `active_tasks_` vector.
       *
       * Also move the new inactive queue into the inactive queue.
       */
      void move_tasks()
      {
        // add new task to the active queue
        active_tasks_.insert(active_tasks_.end(), new_tasks_.begin(), new_tasks_.end());
        new_tasks_.clear();

        // Add inactive task to inactive queue
        inactive_tasks_.insert(inactive_tasks_.end(), new_inactive_tasks_.begin(),
                               new_inactive_tasks_.end());

        // Remove new inactive task from the active queue
        active_tasks_.erase(std::remove_if(active_tasks_.begin(), active_tasks_.end(),
                                           [&](TaskImplPtr t)
                                           {
                                             return std::find(new_inactive_tasks_.begin(),
                                                              new_inactive_tasks_.end(),
                                                              t) !=
                                                    new_inactive_tasks_.end();
                                           }),
                            active_tasks_.end());

        new_inactive_tasks_.clear();

        // remove the completed task from the active queue.
        active_tasks_.erase(std::remove_if(active_tasks_.begin(), active_tasks_.end(),
                                           [&](TaskImplPtr t)
                                           {
                                             return std::find(completed_tasks_.begin(),
                                                              completed_tasks_.end(),
                                                              t) !=
                                                    completed_tasks_.end();
                                           }),
                            active_tasks_.end());
        completed_tasks_.clear();
      }

      /**
       * Mark a task as active.
       */
      void mark_active(const TaskImplPtr &task)
      {
        auto itr = std::find(active_tasks_.begin(), active_tasks_.end(), task);
        assert(itr == active_tasks_.end());

        itr = std::find(inactive_tasks_.begin(), inactive_tasks_.end(), task);
        if (itr != inactive_tasks_.end())
        {
          inactive_tasks_.erase(itr);
        }
        new_tasks_.push_back(task);
      }

      using TaskQueue = std::vector<TaskImplPtr>;

      /**
       * Must be first attribute, so it is destroyed last.
       */
      TaskBuilder builder_;

      /**
       * Task that can (and will) run.
       */
      TaskQueue active_tasks_;

      /**
       * Newly created task that needs to be pushed into the active_task_ vector.
       * This intermediate queue is used to prevent error when inserting while
       * iterating
       * over a vector.
       */
      TaskQueue new_tasks_;

      /**
       * Task waiting for something.
       */
      TaskQueue inactive_tasks_;

      /**
       * Tasks that have just be marked inactive.
       */
      TaskQueue new_inactive_tasks_;

      TaskQueue completed_tasks_;

      TaskImplPtr current_task_;


      friend class BaseEventLoop;
    };
  }
}
