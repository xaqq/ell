#pragma once

#include <memory>

namespace ell
{
  template <typename T>
  class Task;

  template <typename T>
  using TaskPtr = std::shared_ptr<Task<T>>;

  template <typename T>
  class Queue;

  template <typename T>
  class BaseEventLoop;

  namespace details
  {
    class TaskImpl;
    using TaskImplPtr = std::shared_ptr<TaskImpl>;

    class DefaultEventLoop;
    class TaskBuilder;

    /**
     * The event currently used as the backend.
     *
     * The class typedef'd here exposed a richer API
     * than the the `EventLoop` typedef. This
     * extra API is for internal use.
     */
    using EventLoopImpl = details::DefaultEventLoop;
  }

  /**
   * The EventLoop that is used in end-user code.
   */
  using EventLoop = BaseEventLoop<details::DefaultEventLoop>;
}
