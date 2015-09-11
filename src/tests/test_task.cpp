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

TEST(test_task, wait_for)
{
  using namespace ell;
  EventLoop loop;

  bool t1_has_run = false;
  bool t2_has_run = false;

  auto t1_fct = [&]()
  {
    t1_has_run = true;
    return 1;
  };
  auto t2_fct = [&]()
  {
    t2_has_run = true;
    return 2;
  };

  auto main_task = loop.call_soon([&]()
                                  {
                                    auto t1 = loop.call_soon(t1_fct);
                                    auto t2 = loop.call_soon(t2_fct);

                                    ell::wait_for(t1, t2);
                                    ASSERT_TRUE(t1_has_run);
                                    ASSERT_EQ(1, t1->get_result());
                                    ASSERT_EQ(2, t2->get_result());
                                    ASSERT_TRUE(t2_has_run);
                                  });
  loop.run_until_complete(main_task);
}

TEST(test_task, wait_for2)
{
  using namespace ell;
  using namespace std::chrono;

  EventLoop loop;
  auto t1_fct = [&]()
  {
    ell::sleep(milliseconds(750));
  };

  auto t2_fct = [&]()
  {
    ell::sleep(milliseconds(1500));
  };

  auto main_task = loop.call_soon([&]()
                                  {
                                    auto start = system_clock::now();

                                    auto t1 = loop.call_soon(t1_fct);
                                    auto t2 = loop.call_soon(t2_fct);

                                    ell::wait_for(t1, t2);

                                    auto end = system_clock::now();
                                    auto elapsed =
                                        duration_cast<milliseconds>(end - start);
                                    ASSERT_GE(elapsed.count(), 1500);
                                    ASSERT_LE(elapsed.count(), 2000);
                                  });
  loop.run_until_complete(main_task);
}

int main(int ac, char **av)
{
  ell::initialize_logger();
  ::testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
