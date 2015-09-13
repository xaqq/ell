#include <memory>
#include <gtest/gtest.h>
#include "details/intrusive_ptr.hpp"

struct InvalidType
{
};

int global_count      = 0;
int destructor_called = 0;

struct MyObject : public ell::details::RefCounted<MyObject>
{
  ~MyObject()
  {
    destructor_called++;
  }

  void foo()
  {
    global_count++;
  }

  void some_const() const
  {
  }
};

TEST(test_intrusive_ptr, sanity)
{
  using namespace ell;

  // Would not compile because it doesn't derive from RefCounted.
  // details::IntrusivePtr<InvalidType> ptr(nullptr);

  details::IntrusivePtr<MyObject> ptr;
  ASSERT_EQ(nullptr, ptr.get());
  ASSERT_EQ(0, ptr.get_count());
}

TEST(test_intrusive_ptr, simple)
{
  using namespace ell;
  using namespace ell::details;

  destructor_called = 0;
  global_count      = 0;
  {
    IntrusivePtr<MyObject> ptr(new MyObject);

    ASSERT_NE(nullptr, ptr.get());
    ASSERT_EQ(1, ptr.get_count());

    ptr->some_const();
    ptr->foo();
    ASSERT_EQ(1, global_count);
  }
  ASSERT_EQ(1, destructor_called);
}

TEST(test_intrusive_ptr, copy)
{
  using namespace ell;
  using namespace ell::details;

  IntrusivePtr<MyObject> ptr(new MyObject);
  {
    IntrusivePtr<MyObject> cpy(ptr);

    ASSERT_EQ(2, ptr.get_count());
    ASSERT_EQ(2, cpy.get_count());
    ASSERT_EQ(ptr.get(), cpy.get());
  }
  ASSERT_EQ(1, ptr.get_count());
}

int main(int ac, char **av)
{
  ::testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
