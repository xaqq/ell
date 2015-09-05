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
      while (full())
      {
        condvar_.wait();
      }
      storage_.emplace(std::forward<T>(obj));
      condvar_.notify_all();
      ELL_ASSERT(maxsize_ == 0 || size() <= maxsize_, "Too much items in the queue");
    }

    void push(const T &obj)
    {
      while (full())
      {
        condvar_.wait();
      }
      storage_.push(obj);
      condvar_.notify_all();
      ELL_ASSERT(maxsize_ == 0 || size() <= maxsize_, "Too much items in the queue");
    }

    /**
     * Adds an item to the queue, if there is at least one slot
     * available in the queue.
     *
     * If no slots are available, does nothing and returns false.
     */
    bool try_push(T &&obj)
    {
      if (full())
        return false;
      push(std::forward<T>(obj));
      return true;
    }

    bool try_push(const T &obj)
    {
      if (full())
        return false;
      push(obj);
      return true;
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

    /**
     * Returns true if the queue is full, false otherwise.
     *
     * A queue without a maximum size is never full.
     */
    bool full() const
    {
      if (maxsize_ > 0 && size() == maxsize_)
        return true;
      return false;
    }

  private:
    unsigned int maxsize_;
    ConditionVariable condvar_;
    std::queue<T> storage_;
  };
}
