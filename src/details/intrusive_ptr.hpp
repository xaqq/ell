#pragma once

#include <type_traits>
#include "details/ell_assert.hpp"

namespace ell
{
  namespace details
  {

    /**
     * Base class for use for object wishing to
     * be used through an IntrusivePtr pointer.
     *
     * Uses CRTP.
     */
    template <typename T>
    class RefCounted
    {
    public:
      using CountType = unsigned int;

      RefCounted() noexcept : count_(0)
      {
      }

      CountType incr() noexcept
      {
        return ++count_;
      }

      /**
       * Decrement the reference count by 1.
       *
       * If the reference count reach 0,
       * it self delete.
       */
      CountType decr()
      {
        ELL_ASSERT(count_ > 0, "Count would go negative.");
        --count_;
        if (count_ == 0)
        {
          // need support for custom deleter.
          auto self = static_cast<T *>(this);
          delete self;
          return 0;
        }

        return count_;
      }

      CountType count() const noexcept
      {
        return count_;
      }

    private:
      CountType count_;
    };


    /**
     * A wrapper around a intrusively reference counted object.
     *
     * The template parameter must be a class that derives from
     * `RefCounted`.
     */
    template <typename T>
    class IntrusivePtr
    {
    public:
      static_assert(std::is_base_of<RefCounted<T>, T>::value,
                    "Templated type doesn't derive from RefCount.");

      ~IntrusivePtr()
      {
        static_assert(sizeof(IntrusivePtr) == sizeof(T *), "IntrusivePtr is fat.");

        if (rc())
          rc()->decr();
      }

      IntrusivePtr() noexcept : raw_ptr_(nullptr)
      {
      }

      IntrusivePtr(T *ptr) noexcept : raw_ptr_(ptr)
      {
        if (raw_ptr_)
          rc()->incr();
      }

      IntrusivePtr(const IntrusivePtr &o) noexcept : raw_ptr_(o.raw_ptr_)
      {
        if (rc())
          rc()->incr();
      }

      IntrusivePtr(IntrusivePtr &&o) noexcept : raw_ptr_(o.raw_ptr_)
      {
        o.raw_ptr_ = nullptr;
      }

      IntrusivePtr &operator=(const IntrusivePtr &o) noexcept
      {
        raw_ptr_ = o.raw_ptr_;
        if (rc())
          rc()->incr();
      }

      IntrusivePtr &operator=(IntrusivePtr &&o) noexcept
      {
        raw_ptr_   = o.raw_ptr_;
        o.raw_ptr_ = nullptr;
      }

      /**
       * Return the stored pointer.
       */
      T *get() noexcept
      {
        return raw_ptr_;
      }

      /**
       * Return the current ref count for the managed
       * object.
       *
       * If the managed object is nullptr, returns 0.
       */
      typename RefCounted<T>::CountType get_count() const noexcept
      {
        if (rc())
          return rc()->count();
        return 0;
      }

      T *operator->() noexcept
      {
        return raw_ptr_;
      }

      T &operator*() noexcept
      {
        return *raw_ptr_;
      }

    private:
      // Simple helper function to retrieve the base class pointer.
      RefCounted<T> *rc() noexcept
      {
        return static_cast<RefCounted<T> *>(raw_ptr_);
      }

      const RefCounted<T> *rc() const noexcept
      {
        return static_cast<RefCounted<T> *>(raw_ptr_);
      }

      T *raw_ptr_;
    };
  }
}