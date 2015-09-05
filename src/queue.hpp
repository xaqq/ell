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

    /**
     * Remove and return an item from the queue if one is already available.
     *
     * If no item are available, the function returns false. Otherwise
     * it returns true.
     */
    bool try_pop(T &out)
    {
      if (size() > 0)
      {
        out = pop();
        return true;
      }
      return false;
    }

    /**
     * Returns the number of items in the queue.
     */
    size_t size() const
    {
      return storage_.size();
    }

  private:
    ConditionVariable condvar_;
    std::queue<T> storage_;
  };
}
