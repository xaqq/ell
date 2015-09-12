#include <iostream>
#include <gtest/gtest.h>
#include "ell.hpp"
#include "exceptions/cancelled.hpp"

TEST(test_task_cancel, while_yielding)
{
  using namespace ell;
  EventLoop loop;

  auto task = loop.call_soon([]()
                             {
                               while (true)
                                 yield();
                             });
  auto canceller = loop.call_soon([&]()
                                  {
                                    sleep(std::chrono::milliseconds(1000));
                                    task->cancel();
                                  });

  loop.run_until_complete(task);
  ASSERT_THROW(task->get_result();, ex::Cancelled);
}

TEST(test_task_cancel, while_sleeping)
{
  using namespace ell;
  EventLoop loop;

  auto task = loop.call_soon([]()
                             {
                               try
                               {
                                 while (true)
                                   sleep(std::chrono::milliseconds(25));
                               }
                               catch (ex::Cancelled &e)
                               {
                                 return true;
                               }
                               return false;
                             });
  auto canceller = loop.call_soon([&]()
                                  {
                                    sleep(std::chrono::milliseconds(1000));
                                    task->cancel();
                                  });
  loop.run_until_complete(task);
  ASSERT_EQ(true, task->get_result());
}

TEST(test_task_cancel, while_sleeping_a_long_time)
{
  using namespace std::chrono;
  using namespace ell;

  EventLoop loop;
  auto start = system_clock::now();

  // The task we cancelled was scheduled to wait for
  // 5 seconds. the cancel shall take ~1sec (timer for canceller task).
  auto task = loop.call_soon([]()
                             {
                               sleep(std::chrono::milliseconds(5000));
                             });
  auto canceller = loop.call_soon([&]()
                                  {
                                    sleep(std::chrono::milliseconds(1000));
                                    // task->cancel();
                                    loop.cancel_task(task);
                                  });
  loop.run_until_complete(task);
  ASSERT_THROW(task->get_result();, ex::Cancelled);
  auto end     = system_clock::now();
  auto elapsed = duration_cast<milliseconds>(end - start);
  ASSERT_LE(elapsed.count(), 1200); // ~200ms buffer
}

int main(int ac, char **av)
{
  ell::initialize_logger();
  ::testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
