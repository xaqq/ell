#pragma once

#include <queue>
#include "condition_variable.hpp"

namespace ell
{
  /**
   * A queue, for use by coroutines running on the same event loop.
   *
   * @note When the queue is said to "wait" it means means the current task
   * yield and will be resumed later. It does not block the thread.
   * @note This class is not thread safe.
   */
  template <typename T>
  class Queue
  {
  public:
    /**
     * Construct a new queue.
     *
     * If maxsize is less than or equal to
     * zero (the default), the queue size is infinite.
     */
    Queue(int maxsize = -1)
        : maxsize_(maxsize <= 0 ? 0 : maxsize)
    {
    }

    /**
     * Put an item into the queue.
     *
     * If the queue is full, wait until a free slot is available before adding item.
     */
    void push(T &&obj)
    {
      while (maxsize_ > 0 && size() == maxsize_)
      {
        condvar_.wait();
      }
      storage_.emplace(std::forward<T>(obj));
      condvar_.notify_all();
      ELL_ASSERT(maxsize_ == 0 || size() <= maxsize_, "Too much items in the queue");
    }

    void push(const T &obj)
    {
      ELL_DEBUG("Current size: {}. Maxsize: {}", size(), maxsize_);
      while (maxsize_ > 0 && size() == maxsize_)
      {
        condvar_.wait();
      }
      storage_.push(obj);
      condvar_.notify_all();
      ELL_ASSERT(maxsize_ == 0 || size() <= maxsize_, "Too much items in the queue");
    }

    /**
     * Remove and return an item from the queue.
     *
     * If queue is empty, wait until an item is available.
     */
    T pop()
    {
      while (storage_.size() == 0)
      {
        condvar_.wait();
      }

      T tmp = storage_.front();
      storage_.pop();
      condvar_.notify_all();
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
    unsigned int maxsize_;
    ConditionVariable condvar_;
    std::queue<T> storage_;
  };
}
