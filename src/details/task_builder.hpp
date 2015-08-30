#pragma once

#include "details/task_impl.hpp"
#include "task.hpp"

namespace ell
{
  namespace details
  {
    /**
     * And helper class that handles the creation of
     * a `Task` object and the underlying `TaskImpl` object.
     */
    class TaskBuilder
    {
    public:
      TaskBuilder()
          : allocator_(sizeof(Task<int>))
      {
      }

      TaskBuilder(const TaskBuilder &) = delete;
      TaskBuilder &operator=(const TaskBuilder &) = delete;
      TaskBuilder(TaskBuilder &&) = delete;
      TaskBuilder &operator=(TaskBuilder &&) = delete;

      /**
       * Build a `Task<T>` from a user-defined callable.
       */
      template <typename Callable>
      auto build(const Callable &callable) -> TaskPtr<decltype(callable())>
      {
        using ReturnType = decltype(callable());
        static_assert(
            sizeof(Task<ReturnType>) == sizeof(Task<int>) &&
                alignof(Task<ReturnType>) == alignof(Task<int>),
            "Implementation details of Task<T> changed and depends too much on T.");

        auto memory                    = allocator_.malloc();
        Task<ReturnType> *task_raw_ptr = new (memory) Task<ReturnType>(callable);
        TaskPtr<ReturnType> task =
            TaskPtr<ReturnType>(task_raw_ptr, [this](Task<ReturnType> *ptr)
                                {
                                  ptr->~Task<ReturnType>();
                                  allocator_.free(ptr);
                                });
        return task;
      }

    private:
      boost::pool<> allocator_;
    };
  }
}
