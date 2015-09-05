#define ELL_HAVE_VALGRIND 1

#ifdef ELL_HAVE_VALGRIND
#include <unordered_map>
#include <valgrind/valgrind.h>
#endif
#include <boost/coroutine/stack_allocator.hpp>

#define ELL_COROUTINE_STACK_SIZE (1024 * 65)

namespace ell
{
  namespace details
  {
    struct CoroutineStackAllocator
    {
      static boost::pool<> &pool()
      {
        static boost::pool<> p(ELL_COROUTINE_STACK_SIZE);
        return p;
      }
    };
    /**
     * Implements the concept of a stack-allocator.
     * If valgrind was found during the build, this allocator
     * will register the stack to valgrind.
     *
     * This will help avoid tons of false positive memory error.
     */
    class valgrind_stack_allocator
    {
      boost::coroutines::stack_allocator allocator;
#ifdef ELL_HAVE_VALGRIND
      std::unordered_map<void *, unsigned> stack_ids;
#endif

    public:
      void allocate(boost::coroutines::stack_context &sc, std::size_t size)
      {
        if (size == ELL_COROUTINE_STACK_SIZE)
        {
          auto limit = CoroutineStackAllocator::pool().malloc();
          sc.size    = size;
          sc.sp      = static_cast<char *>(limit) + sc.size;
        }
        else
        {
          allocator.allocate(sc, size);
        }
#ifdef ELL_HAVE_VALGRIND
        auto res = stack_ids.insert(std::make_pair(
            sc.sp, VALGRIND_STACK_REGISTER(sc.sp, (((char *)sc.sp) - sc.size))));
        (void)res;
        assert(res.second);
#endif
      }

      void deallocate(boost::coroutines::stack_context &sc)
      {
#ifdef ELL_HAVE_VALGRIND
        auto id = stack_ids.find(sc.sp);
        assert(id != stack_ids.end());
        VALGRIND_STACK_DEREGISTER(id->second);
        stack_ids.erase(id);
#endif
        if (sc.size == ELL_COROUTINE_STACK_SIZE)
        {
          void *limit = static_cast<char *>(sc.sp) - sc.size;
          CoroutineStackAllocator::pool().free(limit);
        }
        else
        {
          allocator.deallocate(sc);
        }
      }
    };
  }
}
