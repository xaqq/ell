#include <iostream>
#include <gtest/gtest.h>
#include "ell.hpp"
#include "exceptions/cancelled.hpp"

TEST(test_task, simple_yield)
{
  using namespace ell;
  EventLoop loop;

  int count    = 0;
  bool has_run = false;

  auto task2 = loop.call_soon([&]()
                              {
                                for (int i = 0; i < 5; ++i)
                                {
                                  ASSERT_TRUE(has_run); // make sure that has run first.
                                  count--;
                                  ASSERT_EQ(0, count);
                                  yield();
                                }
                              });
  auto task = loop.call_soon([&]()
                             {
                               for (int i = 0; i < 5; ++i)
                               {
                                 has_run = true;
                                 ASSERT_EQ(0, count);
                                 count++;
                                 yield();
                               }
                             });
  loop.run_until_complete(task);
  ASSERT_EQ(0, count);
}

TEST(test_task, chained)
{
  using namespace ell;

  EventLoop loop;
  int count = 0;
  auto task = loop.call_soon([&]()
                             {
                               ASSERT_EQ(0, count);
                               yield([&]()
                                     {
                                       ASSERT_EQ(0, count);
                                       count++;

                                       yield([&]()
                                             {
                                               ASSERT_EQ(1, count);
                                               count++;

                                               yield([&]()
                                                     {
                                                       ASSERT_EQ(2, count);
                                                       count++;
                                                       yield();
                                                     });
                                             });
                                     });
                             });
  loop.run_until_complete(task);
  ASSERT_EQ(3, count);
}

int main(int ac, char **av)
{
  ell::initialize_logger();
  ::testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
