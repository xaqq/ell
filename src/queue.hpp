#pragma once

#include <queue>
#include "condition_variable.hpp"

namespace ell
{
  /**
   * A non thread safe queue.
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
