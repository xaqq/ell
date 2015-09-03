#pragma once

#include <queue>
#include "condition_variable.hpp"

namespace ell
{
  /**
   * A coroutine-aware (non thread-safe) queue.
   *
   * `pop`ing from the queue will put the coroutine
   * to sleep until something can be popped.
   */
  template <typename T>
  class Queue
  {
  public:
    void push(T &&obj)
    {
      storage_.emplace(std::forward<T>(obj));
      condvar_.notify_all();
    }

    T pop()
    {
      while (storage_.size() == 0)
      {
        condvar_.wait();
      }

      T tmp = storage_.front();
      storage_.pop();
      return tmp;
    }

  private:
    ConditionVariable condvar_;
    std::queue<T> storage_;
  };
}
