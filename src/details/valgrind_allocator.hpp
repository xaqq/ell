#define ELL_HAVE_VALGRIND 1

#ifdef ELL_HAVE_VALGRIND
#include <unordered_map>
#include <valgrind/valgrind.h>
#endif
#include <boost/coroutine/stack_allocator.hpp>

namespace ell
{
  namespace details
  {
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
        allocator.allocate(sc, size);
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
        allocator.deallocate(sc);
      }
    };
  }
}
