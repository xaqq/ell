#pragma once

#include <unordered_set>
#include "ell.hpp"
#include "base_event_loop.hpp"
#include "details/task_builder.hpp"
#include "condition_variable"
#include "details/task_sleep.hpp"
#include <iostream>

namespace ell
{
  namespace details
  {
    class DefaultEventLoop : public BaseEventLoop<DefaultEventLoop>
    {
    public:
      /**
       * Should not be used by end-user.
       */
      TaskImplPtr current_task()
      {
        return current_task_;
      }

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
        handle_wait_handlers();
        move_tasks();
        wake_tasks();
        try_to_sleep();

        for (auto &task : active_)
        {
          current_task_ = task;
          task->resume();

          if (task->is_complete())
          {
            std::cout << "Task complete !" << std::endl;
            task_completed(task);
          }
          else
          {
            ELL_TRACE("Task not complete. {}");
          }
        }
        current_task_ = nullptr;
      }

      template <typename Callable>
      auto yield_impl(const Callable &callable) -> decltype(callable())
      {
        auto task = call_soon(callable);

        attach_wait_handler(task->impl_.wait_handler(), current_task_);
        current_task_->suspend();

        return task->get_result();
      }

      template <typename Duration>
      void sleep_current_task_impl(const Duration &duration)
      {
        TaskSleep task(current_task_, duration);
        attach_wait_handler(task.wait_handler(), current_task_);

        sleep_tasks_.push_back(task);

        current_task_->suspend();
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
        active_.insert(new_tasks_.begin(), new_tasks_.end());
        new_tasks_.clear();

        // remove the completed task from the active queue.
        for (auto &completed : completed_tasks_)
        {
          active_.erase(completed);
        }
        completed_tasks_.clear();
      }

      /**
       * Wake tasks that were sleeping if the time elapsed
       */
      void wake_tasks()
      {
        auto now = std::chrono::system_clock::now();
        sleep_tasks_.erase(std::remove_if(sleep_tasks_.begin(), sleep_tasks_.end(),
                                          [&](TaskSleep &task)
                                          {
                                            if (now >= task.until())
                                            {
                                              std::cout << "REMOVING" << std::endl;
                                              detach_wait_handler(task.wait_handler());
                                              return true;
                                            }
                                            return false;
                                          }),
                           sleep_tasks_.end());
      }

      /**
       * We may to able to really sleep the thread.
       * If no task are active, we check the sleep_tasks_ and sleep for
       * shortest duration.
       */
      void try_to_sleep()
      {
        using namespace std::chrono;

        if (active_.size() > 0 || sleep_tasks_.size() == 0)
          return;

        system_clock::time_point shortest = system_clock::time_point::max();
        for (auto &sleep_task : sleep_tasks_)
        {
          if (shortest > sleep_task.until())
          {
            shortest = sleep_task.until();
          }
        }

        std::this_thread::sleep_until(shortest);
      }

      /**
       * Change the status of the tasks based on
       * the wait handlers.
       */
      void handle_wait_handlers()
      {
        ELL_ASSERT(current_task_ == nullptr, "Called while running a task.");
        for (auto &task : dirty_tasks_)
        {
          if (task->wait_count() == 0)
          {
            inactive_.erase(task);
            active_.insert(task);
          }
          else
          {
            active_.erase(task);
            inactive_.insert(task);
          }
        }
        dirty_tasks_.clear();
      }

      /**
       * Mark a task as completed.
       * Shall happen during a loop iteration.
       */
      void task_completed(const TaskImplPtr &task)
      {
        ELL_ASSERT(current_task_ && current_task_ == task, "Unexpected task "
                                                           "marked as complete.");
        ELL_TRACE("Task completed.");

        detach_wait_handler(task->wait_handler());
        completed_tasks_.push_back(task);
      }

    public:
      void attach_wait_handler(WaitHandler &handler, const TaskImplPtr &task)
      {
        ELL_TRACE("Attaching handler {} to task {}.", handler.id(), task.id());
        if (task->wait_count() == 0)
          dirty_tasks_.insert(task);
        task->incr_wait_count();
        handler.tasks_.push_back(task);
      }

      /**
       * Remove a WaitHandler from tasks that were waiting on it.
       * Mark the corresponding task dirty.
       */
      void detach_wait_handler(WaitHandler &handler)
      {
        ELL_TRACE("Detaching WaitHandler with id: {}. Tasks waiting on this handler: {}",
                  handler.id(), handler.waiter_count());
        for (auto &t : handler.tasks_)
        {
          t->decr_wait_count();
          if (t->wait_count() == 0)
          {
            ELL_TRACE("Marking task {} dirty.", t->id());
            dirty_tasks_.insert(t);
          }
        }
      }

    private:
      using TaskQueue    = std::vector<TaskImplPtr>;
      using TaskQueueNew = std::unordered_set<TaskImplPtr>;

      /**
       * Must be first attribute, so it is destroyed last.
       */
      TaskBuilder builder_;

      TaskQueueNew active_;
      TaskQueueNew inactive_;

      /**
       * Newly created task that needs to be pushed into the active_task_ vector.
       * This intermediate queue is used to prevent error when inserting while
       * iterating
       * over a vector.
       */
      TaskQueue new_tasks_;

      TaskQueue completed_tasks_;

      TaskImplPtr current_task_;

      /**
       * Tasks whose WaitHandler list changed
       * since we loop iteration.
       */
      TaskQueueNew dirty_tasks_;
      std::multimap<WaitHandler, TaskImplPtr> whandler_tasks_;

      /**
       * List of `TaskSleep`
       */
      std::vector<TaskSleep> sleep_tasks_;

      friend class BaseEventLoop;
    };
  }
}
