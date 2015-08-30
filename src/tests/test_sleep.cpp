#include <iostream>
#include <gtest/gtest.h>
#include "ell.hpp"

TEST(test_sleep, task_can_sleep)
{
  ell::EventLoop loop;

  auto test_impl = []()
  {
    using namespace std::chrono;
    auto start = system_clock::now();

    ell::sleep(milliseconds(4000));

    auto end     = system_clock::now();
    auto elapsed = duration_cast<milliseconds>(end - start);

    ASSERT_GE(elapsed.count(), 4000);
  };

  auto task = loop.call_soon(test_impl);
  loop.run_until_complete(task);
}

TEST(test_sleep, concurrent_sleep)
{
  using namespace std::chrono;

  ell::EventLoop loop;
  auto start           = system_clock::now();
  auto sleep_coroutine = [](int mssleep)
  {
    ell::sleep(milliseconds(mssleep));
  };
  auto t1 = loop.call_soon(std::bind(sleep_coroutine, 1000));
  auto t2 = loop.call_soon(std::bind(sleep_coroutine, 2000));
  auto t3 = loop.call_soon(std::bind(sleep_coroutine, 3000));

  loop.run_until_complete(t3);
  auto end     = system_clock::now();
  auto elapsed = duration_cast<milliseconds>(end - start);

  // Give a ~1sec delay max. The goal is to
  // demonstrate that we wont wait 6seconds.
  ASSERT_LE(elapsed.count(), 4000);
  ASSERT_GE(elapsed.count(), 3000);
}

int main(int ac, char **av)
{
  ::testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
