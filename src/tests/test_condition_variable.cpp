#include <iostream>
#include <gtest/gtest.h>
#include "ell.hpp"
#include "condition_variable.hpp"

TEST(test_cond_var, simple_wait_notify)
{
  using namespace std::chrono;

  auto start = system_clock::now();
  ell::EventLoop loop;
  ell::ConditionVariable cond;

  auto pusher = [&]() -> void
  {
    ell::sleep(std::chrono::milliseconds(2500));
    cond.notify_all();
  };

  auto popper = [&]() -> void
  {
    cond.wait();
  };

  auto pusher_task = loop.call_soon(pusher);
  auto pop_task    = loop.call_soon(popper);

  // Will take 2500 ms.
  loop.run_until_complete(pop_task);

  auto end     = system_clock::now();
  auto elapsed = duration_cast<milliseconds>(end - start);
  ASSERT_GE(elapsed.count(), 2500);
}

int main(int ac, char **av)
{
  ::testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}