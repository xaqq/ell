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
        auto promise     = new std::promise<ReturnType>();

        ret->promise_ = TypeErasedUniquePtr(
            reinterpret_cast<uint8_t *>(promise), [](uint8_t *ptr)
            {
              delete (reinterpret_cast<std::promise<ReturnType> *>(ptr));
            });

        // Allocated space for a future<T> and then move the promise's future
        // into our future erased container
        auto future  = new std::future<ReturnType>();
        *future      = std::move(promise->get_future());
        ret->future_ = TypeErasedUniquePtr(
            reinterpret_cast<uint8_t *>(future), [](uint8_t *ptr)
            {
              delete (reinterpret_cast<std::future<ReturnType> *>(ptr));
            });

        setup_coroutine(ret, callable);
        return ret;
      }

      template <typename Callable>
      static void setup_coroutine(TaskImplPtr task, const Callable &callable)
      {
        // We must now setup the boost coroutine object.
        // We will wrap the user callable into a coroutine, adding some
        // code to handles return values, exceptions, and initialization.
        task->coroutine_ = CoroutineCall(
            [&](CoroutineYield &yield)
            {
              // Perform some initialization task, then yield.

              // Save a pointer to self while the lambda capture has a correct
              // reference to "task".
              TaskImpl *self =
                  task.get(); // While the coroutine is alive, the TaskImpl obj
              // is alive too.
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
        task->coroutine_();
      }

      /**
       * Resume the task, effectively calling the coroutine
       * so that it runs.
       */
      void resume()
      {
        std::cout << "Resuming task !" << std::endl;
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
      using TypeErasedUniquePtr = std::unique_ptr<uint8_t, void (*)(uint8_t *)>;

      TypeErasedUniquePtr promise_;
      TypeErasedUniquePtr future_;

      using CoroutineCall = boost::coroutines::symmetric_coroutine<void>::call_type;
      using CoroutineYield =
          boost::coroutines::symmetric_coroutine<void>::yield_type;

      /**
       * The user-code that will be run is wrapped into a Coroutine object.
       */
      CoroutineCall coroutine_;

      /**
       * A pointer to the yield object that the boost coroutine gives us.
       */
      CoroutineYield *yield_;

    public:
      /**
       * List of task that depends on this task.
       */
      std::vector<TaskImplPtr> dependants_;
    };
  }
}