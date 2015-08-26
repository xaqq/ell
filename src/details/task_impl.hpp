#pragma once

#include <memory>
#include <future>
#include <cassert>
#include <iostream>
#include <boost/coroutine/coroutine.hpp>
#include "ell_fwd.hpp"
#include "valgrind_allocator.hpp"

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
      /**
       * This is a deleter that shall never be called.
       *
       * It's raison d'etre is because construction a std::unique_ptr with a custom
       * deleter needs a non-full deleter function.
       * However this deleter will never be called because the unique_ptr will
       * be replaced by one with a compliant deleter.
       */
      static void dummy_deleter(uint8_t *)
      {
        assert(0);
      }

      TaskImpl()
          : promise_(nullptr, &dummy_deleter)
          , future_(nullptr, &dummy_deleter)
          , yield_(nullptr)
      {
      }

      TaskImpl(const TaskImpl &) = delete;

    public:
      /**
       * Returns the result of the task.
       */
      template <typename T>
      T get_result()
      {
        auto future = reinterpret_cast<std::future<T> *>(future_.get());
        return future->get();
      }

      /**
       * Create a new TaskImpl object, wrap it into a shared pointer
       * and returns it.
       *
       * It create the correct std::promise<T> and std::future<T> and move them
       * into type-erased unique_ptr with proper deleter.
       */
      template <typename Callable>
      static TaskImplPtr create(const Callable &callable)
      {
        using ReturnType = decltype(callable());
        auto ret         = TaskImplPtr(new TaskImpl());

        ret->setup_promise<ReturnType>();
        ret->setup_future<ReturnType>();
        ret->setup_coroutine(callable);
        return ret;
      }

      /**
       * Resume the task, effectively calling the coroutine
       * so that it runs.
       */
      void resume()
      {
        // std::cout << "Resuming task !" << std::endl;
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

    private:
      /**
       * Configure the internal promise.
       */
      template <typename ReturnType>
      void setup_promise()
      {
        using PromiseType = std::promise<ReturnType>;
        // Make sure the storage optimization we do can be used and is correct.
        static_assert(
            sizeof(PromiseType) == sizeof(std::promise<int>) &&
                alignof(PromiseType) == alignof(std::promise<int>),
            "Cannot use buffer optimisation. Assertion about promise memory layout "
            "are wrong.");

        auto promise = new (&promise_storage_) PromiseType();
        promise_ =
            TypeErasedUniquePtr(reinterpret_cast<uint8_t *>(promise), [](uint8_t *ptr)
                                {
                                  reinterpret_cast<PromiseType *>(ptr)->~PromiseType();
                                });
      }

      /**
       * Configure the internal future object.
       */
      template <typename ReturnType>
      void setup_future()
      {
        using FutureType = std::future<ReturnType>;
        static_assert(sizeof(FutureType) == sizeof(std::future<int>) &&
                          alignof(FutureType) == alignof(std::future<int>),
                      "Cannot use buffer optimisation. Assertion about future "
                      "memory layout are wrong.");
        assert(promise_);

        auto future = new (&future_storage_) FutureType();
        *future = std::move(
            reinterpret_cast<std::promise<ReturnType> *>(promise_.get())->get_future());
        future_ =
            TypeErasedUniquePtr(reinterpret_cast<uint8_t *>(future), [](uint8_t *ptr)
                                {
                                  reinterpret_cast<FutureType *>(ptr)->~FutureType();
                                });
      }

      /**
       * Create the coroutine object and configure it to run the Callable.
       */
      template <typename Callable>
      void setup_coroutine(const Callable &callable)
      {
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

              // We'll be there after the loop scheduler run the Task
              // for the first time.
              auto promise = reinterpret_cast<std::promise<decltype(callable())> *>(
                  self->promise_.get());
              try
              {
                promise->set_value(callable());
              }
              catch (const std::exception &)
              {
                promise->set_exception(std::current_exception());
              }
            },
            boost::coroutines::attributes(), valgrind_stack_allocator());
        // Run the coroutine to perform initialization task.
        coroutine_();
      }

      using TypeErasedUniquePtr = std::unique_ptr<uint8_t, void (*)(uint8_t *)>;

      TypeErasedUniquePtr promise_;
      TypeErasedUniquePtr future_;

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

      /**
       * Storage we use for the promise object.
       * We use placement new to this memory location for performance reason.
       */
      std::aligned_storage<sizeof(std::promise<int>), alignof(std::promise<int>)>::type
          promise_storage_;

      /**
       * Ditto for future.
       */
      std::aligned_storage<sizeof(std::future<int>), alignof(std::future<int>)>::type
          future_storage_;

    public:
      /**
       * List of task that depends on this task.
       */
      std::vector<TaskImplPtr> dependants_;
    };
  }
}