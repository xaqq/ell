#pragma once

#include <memory>
#include <future>
#include <cassert>
#include <iostream>
#include <boost/coroutine/coroutine.hpp>
#include <boost/pool/pool.hpp>
#include "ell_fwd.hpp"
#include "valgrind_allocator.hpp"
#include "details/result_holder.hpp"

namespace ell
{
  namespace details
  {

    struct Foo
    {
      static boost::pool<> &pool()
      {
        static boost::pool<> p(200);
        return p;
      }
    };
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
      TaskImpl() :
              yield_(nullptr)
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
        return result_.get<T>();
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
        auto mem         = Foo::pool().malloc();
        auto task        = new (mem) TaskImpl();

        auto ret = TaskImplPtr(task, [](TaskImpl *ptr)
                               {
                                 ptr->~TaskImpl();
                                 Foo::pool().free(ptr);
                               });

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
       * Create the coroutine object and configure it to run the Callable.
       */
      template <typename Callable>
      void setup_coroutine(const Callable &callable)
      {
        auto attr = boost::coroutines::attributes();
        attr.size = 1024 * 4;

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
                result_.store(callable());
              }
              catch (const std::exception &)
              {
                result_.store_exception(std::current_exception());
              }
            },
            attr, valgrind_stack_allocator());

        // boost::coroutines::attributes(), valgrind_stack_allocator());
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
    public:
      /**
       * List of task that depends on this task.
       */
      std::vector<TaskImplPtr> dependants_;
    };
  }
}