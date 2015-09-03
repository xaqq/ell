#pragma once

#include <limits>
#include <cstdint>


namespace ell
{
  namespace details
  {
    /**
     * A WaitHandler is used to refer to everything a
     * coroutine could be waiting for.
    *
     * Coroutine activity is tracker through WaitHandler.
     * A WaitHandler is attached to a task by calling
     *
     *
     */
    class WaitHandler
    {
    public:
      WaitHandler()
      {
        id_ = next_id();
      }

      WaitHandler(const WaitHandler &o)
      {
        id_    = o.id();
        tasks_ = o.tasks_;
      }

      WaitHandler(WaitHandler &&o)
      {
        id_    = o.id();
        tasks_ = std::move(o.tasks_);

        o.id_ = 0;
      }

      WaitHandler &operator=(const WaitHandler &o)
      {
        id_    = o.id();
        tasks_ = o.tasks_;
        return *this;
      }

      WaitHandler &operator=(WaitHandler &&o)
      {
        id_    = o.id();
        tasks_ = std::move(o.tasks_);

        o.id_ = 0;
        return *this;
      }

      bool operator==(const WaitHandler &o) const
      {
        return id_ == o.id_;
      }

      bool operator<(const WaitHandler &o) const
      {
        return id_ < o.id_;
      }

      uint64_t id() const
      {
        return id_;
      }

      /**
       * Returns the number of tasks waiting for
       * this is handler.
       */
      size_t waiter_count() const
      {
        return tasks_.size();
      }

      /**
       * Reset the handler, clearing the list of
       * tasks waiting for this handler.
       */
      void reset()
      {
        tasks_.clear();
      }

      std::vector<TaskImplPtr> &tasks()
      {
        return tasks_;
      }

    private:
      uint64_t id_;
      std::vector<TaskImplPtr> tasks_;

      static uint64_t next_id()
      {
        static uint64_t count = 0;
        if (count == std::numeric_limits<uint64_t>::max())
        {
          ELL_ASSERT(0, "Out of ids");
        }
        return ++count;
      }

      friend class std::hash<WaitHandler>;
    };
  }
}

// Add specialization to the `std` namespace.
// This is legal as per the Standard.

namespace std
{
  template <>
  struct hash<ell::details::WaitHandler>
  {
    typedef ell::details::WaitHandler argument_type;
    typedef std::size_t result_type;

    result_type operator()(argument_type const &wh) const
    {
      static_assert(std::is_same<result_type, uint64_t>::value, "Not same type");
      return wh.id_;
    }
  };
}
