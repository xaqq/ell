#include "details/result_holder.hpp"
#include <iostream>
#include <gtest/gtest.h>

using namespace ell::details;

static int copying_call = 0;
static int moving_call  = 0;

struct BigObj
{
  BigObj()
      : s_({42})
  {
    ptr_ = malloc(1024);
  }

  ~BigObj()
  {
    free(ptr_);
  }

  BigObj(const BigObj &o)
      : s_(o.s_)
  {
    copying_call++;
    ptr_ = malloc(1024);
  }

  BigObj(BigObj &&o)
      : s_(std::move(o.s_))
  {
    moving_call++;
    ptr_   = o.ptr_;
    o.ptr_ = nullptr;
  }

  void *ptr_;
  std::array<uint8_t, 1024> s_;
};

struct BigObjNotCopyable
{
  BigObjNotCopyable()
      : values_{rand()} {};
  BigObjNotCopyable(const BigObjNotCopyable &o) = delete;
  BigObjNotCopyable(BigObjNotCopyable &&o)
      : values_(std::move(o.values_))
  {
  }

  std::array<int, 512> values_;
};

TEST(test_result_holder, store_big_obj2)
{
  ResultHolder rh;
  BigObjNotCopyable o;
  // rh.store(o); Will not compile.
  rh.store(std::move(o));

  BigObjNotCopyable ret(rh.get<BigObjNotCopyable>());
  ASSERT_EQ(o.values_, ret.values_);
}

TEST(test_result_holder, store_big_obj_move)
{
  copying_call = 0;
  moving_call  = 0;

  ResultHolder rh;
  BigObj o;
  rh.store(std::move(o));

  BigObj ret(rh.get<BigObj>());
  ASSERT_EQ(o.s_, ret.s_);

  ASSERT_EQ(0, copying_call);
  ASSERT_EQ(2, moving_call);
}

TEST(test_result_holder, store_big_obj)
{
  copying_call = 0;
  moving_call  = 0;

  ResultHolder rh;
  BigObj o;
  rh.store(o);

  BigObj ret(rh.get<BigObj>());
  ASSERT_EQ(o.s_, ret.s_);

  ASSERT_EQ(1, copying_call);
  ASSERT_EQ(1, moving_call);
}

TEST(test_result_holder, store_int)
{
  ResultHolder rh;
  int n = 42;

  rh.store(n);
  ASSERT_EQ(n, rh.get<int>());
}

TEST(test_result_holder, store_int_rvalue)
{
  ResultHolder rh;

  rh.store(1337);
  ASSERT_EQ(1337, rh.get<int>());
}

TEST(test_result_holder, store_int_const_ref)
{
  ResultHolder rh;
  const int n = 42;

  rh.store(n);
  ASSERT_EQ(n, rh.get<int>());
}

TEST(test_result_holder, valid)
{
  ResultHolder rh;
  ResultHolder rh2;

  ASSERT_FALSE(rh.valid());
  ASSERT_FALSE(rh2.valid());

  rh.store(21);
  ASSERT_TRUE(rh.valid());

  try
  {
    throw std::runtime_error("Oops");
  }
  catch (...)
  {
    rh2.store_exception(std::current_exception());
  }
  ASSERT_TRUE(rh2.valid());

  rh.get<int>();
  ASSERT_FALSE(rh.valid());

  try
  {
    rh2.get<int>();
  }
  catch (...)
  {
  }
  ASSERT_FALSE(rh.valid());
}

TEST(test_result_holder, store_exception)
{
  ResultHolder rh;

  try
  {
    throw std::runtime_error("Oops");
  }
  catch (...)
  {
    rh.store_exception(std::current_exception());
  }
  ASSERT_THROW(rh.get<int>(), std::runtime_error);
}

TEST(test_result_holder, store_void)
{
  ResultHolder rh;

  rh.store();
  rh.get<void>();
}

int main(int ac, char **av)
{
  ::testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
