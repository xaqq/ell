//
// Created by xaqq on 8/30/15.
//

#include "details/result_holder.hpp"
#include <cassert>
#include <iostream>

using namespace ell::details;

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
    std::cout << "Copying" << std::endl;
    ptr_ = malloc(1024);
  }

  BigObj(BigObj &&o)
      : s_(std::move(o.s_))
  {
    std::cout << "Moving !" << std::endl;
    ptr_   = o.ptr_;
    o.ptr_ = nullptr;
  }

  void *ptr_;
  std::array<uint8_t, 1024> s_;
};

static void test_local_storage()
{
  ResultHolder rh;
  int n = 42;
  rh.store(n);
  assert(rh.get<int>() == n);
}

static void test_heap_storage()
{
  ResultHolder rh;
  BigObj o;
  // rh.store(std::move(o));
  rh.store(o);

  BigObj ret(rh.get<BigObj>());
  assert(o.s_ == ret.s_);
}

// tests the implementation of the internal ResultHolder class.
int main(int, char **)
{
  std::cout << "sizeof ResultHolderImpl<32, 8>: " << sizeof(ResultHolder) << std::endl;
  test_local_storage();
  test_heap_storage();
}
