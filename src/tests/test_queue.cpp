#include <iostream>
#include <gtest/gtest.h>
#include "ell.hpp"
#include "queue.hpp"

TEST(test_queue, simple_push_pop)
{
  using namespace std::chrono;

  ell::EventLoop loop;
  ell::Queue<int> queue;
  auto start = system_clock::now();

  auto pusher = [&]() -> void
  {
    ell::sleep(std::chrono::milliseconds(2500));
    queue.push(42);
    queue.push(21);
  };

  auto popper = [&]() -> int
  {
    int v1 = queue.pop();
    [&]()
    {
      ASSERT_EQ(42, v1);
    }();

    // We should have waited for 2.5 second.
    auto end     = system_clock::now();
    auto elapsed = duration_cast<milliseconds>(end - start);
    [&]()
    {
      ASSERT_GE(elapsed.count(), 2500);
    }();

    // Second pop should be instant.
    int v2 = queue.pop();
    [&]()
    {
      ASSERT_EQ(21, v2);
    }();

    auto end2 = system_clock::now();
    elapsed = duration_cast<milliseconds>(end2 - end);
    [&]()
    {
      ASSERT_LE(elapsed.count(), 5);
    }(); // We allow ~5ms if CPU is slow.

    return v1;
  };

  auto pusher_task = loop.call_soon(pusher);
  auto pop_task    = loop.call_soon(popper);

  // The coroutine popper will wait until an `int` becomes
  // available in the queue.
  loop.run_until_complete(pop_task);

  ASSERT_EQ(42, pop_task->get_result());
}

int main(int ac, char **av)
{
  ::testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
