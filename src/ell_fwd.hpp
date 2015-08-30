#pragma once

#include <memory>

namespace ell
{
  template <typename T>
  class Task;

  template <typename T>
  using TaskPtr = std::shared_ptr<Task<T>>;

  namespace details
  {
    class TaskImpl;
    using TaskImplPtr = std::shared_ptr<TaskImpl>;
    class DefaultEventLoop;

    class TaskBuilder;
  }

  using EventLoop = details::DefaultEventLoop;
}
