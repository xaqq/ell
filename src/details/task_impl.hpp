#pragma once

#include <memory>
#include <future>
#include <cassert>
#include <iostream>
#include <boost/coroutine/coroutine.hpp>
#include <boost/pool/pool.hpp>
#include <unordered_set>
#include "ell_fwd.hpp"
#include "valgrind_allocator.hpp"
#include "details/result_holder.hpp"
#include "details/wait_handler.hpp"
#include "details/ell_log.hpp"

namespace ell
{
  namespace details
  {
    /**
     * The class representing a user task from inside the library.
     *
     * This type provides type-erasure so the event loop can deal we
     * various task that do not returns the same thing.
     *
     * @note The constructor is private, and an instance shall be created
     * using the `create()` helper static method. This is to ensure proper
     * setup of the object.
     *
     * @note The task uses std::promise and std::future to store its result.
     * These 2 objects are type erased. The future corresponding to the
     * promise is retrieve once when the task is create.
     */
    class TaskImpl
    {
    public:
      template <typename Callable>
      TaskImpl(const Callable &callable)
          : yield_(nullptr)
          , wait_count_(0)
          , id_(next_id())
          , is_active_(false)
      {
        setup_coroutine(callable);
      }

      TaskImpl(const TaskImpl &) = delete;
      TaskImpl(TaskImpl &&) = delete;

      TaskImpl &operator=(const TaskImpl &) = delete;
      TaskImpl &operator=(TaskImpl &&) = delete;

      /**
       * Returns the result of the task.
       */
      template <typename T>
      T get_result()
      {
        return result_.get<T>();
      }

      /**
       * Resume the task, effectively calling the coroutine
       * so that it runs.
       */
      void resume()
      {
        ELL_TRACE("Resuming task {}", id_);
        coroutine_();
      }

      void suspend()
      {
        (*yield_)();
      }

      bool is_complete() const
      {
        return !coroutine_;
      }

      /**
       * Mark the task active.
       * This is only used as cache and is maintained by the event loop.
       */
      void set_active(bool val)
      {
        is_active_ = val;
      }

      /**
       * Mark the task active.
       * This is only used as cache and is maintained by the event loop.
       */
      bool is_active() const
      {
        return is_active_;
      }

      WaitHandler &wait_handler() const
      {
        return wait_handler_;
      }

      uint64_t id() const
      {
        return id_;
      }

      /**
       * Returns the number of WaitHandler we are waiting for.
       */
      unsigned int wait_count() const
      {
        return wait_count_;
      }

      /**
       * Increment the current wait_count by 1.
       */
      void incr_wait_count()
      {
        wait_count_++;
      }

      /**
       * Decrement the current wait_count by 1.
       */
      void decr_wait_count()
      {
        ELL_ASSERT(wait_count_ > 0, "wait_count cannot be negative.");
        wait_count_--;
      }

    private:
      template <typename Callable>
      auto wrap_user_call(const Callable &callable)
          -> std::enable_if_t<!std::is_same<void, decltype(callable())>::value, void>
      {
        result_.store(callable());
      }

      template <typename Callable>
      auto wrap_user_call(const Callable &callable)
          -> std::enable_if_t<std::is_same<void, decltype(callable())>::value, void>
      {
        callable();
        result_.store();
      }

      /**
       * Create the coroutine object and configure it to run the Callable.
       */
      template <typename Callable>
      void setup_coroutine(const Callable &callable)
      {
        auto attr = boost::coroutines::attributes();
        attr.size = ELL_COROUTINE_STACK_SIZE;

        // We must now setup the boost coroutine object.
        // We will wrap the user callable into a coroutine, adding some
        // code to handles return values, exceptions, and initialization.
        coroutine_ = CoroutineCall(
            [&](CoroutineYield &yield)
            {
              // Perform some initialization task, then yield.

              // Save a pointer to self while the lambda capture has a correct
              // reference to "task".
              TaskImpl *self = this; // While the coroutine is alive, the TaskImpl
                                     // obj is alive too.
              self->yield_ = &yield;
              yield();

              try
              {
                wrap_user_call(callable);
              }
              catch (const std::exception &)
              {
                result_.store_exception(std::current_exception());
              }
            },
            attr, valgrind_stack_allocator());
        // Run the coroutine to perform initialization task.
        coroutine_();
      }

      using CoroutineCall  = boost::coroutines::symmetric_coroutine<void>::call_type;
      using CoroutineYield = boost::coroutines::symmetric_coroutine<void>::yield_type;

      /**
       * The user-code that will be run is wrapped into a Coroutine object.
       */
      CoroutineCall coroutine_;

      /**
       * A pointer to the yield object that the boost coroutine gives us.
       */
      CoroutineYield *yield_;

      ResultHolder result_;

      /**
       * Wait handler for this task.
       */
      mutable WaitHandler wait_handler_;

      unsigned int wait_count_;

      uint64_t id_;

      bool is_active_;

      /**
       * Gives a unique ID to each task.
       */
      static uint64_t next_id()
      {
        static uint64_t count = 0;
        if (count == std::numeric_limits<uint64_t>::max())
        {
          ELL_ASSERT(0, "Running out of ids.");
        }
        return ++count;
      }
    };
  }
}
