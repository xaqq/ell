#include <iostream>
#include <gtest/gtest.h>
#include "ell.hpp"

TEST(test_signal_handler, install)
{
  using namespace ell;
  using namespace ell::details;

  int count = 0;
  SignalHandler sh(SIGINT, [&](int) -> void
                   {
                     count++;
                   });
  raise(SIGINT);
  ASSERT_EQ(1, count);
}

TEST(test_signal_handler, install_invalid)
{
  using namespace ell;
  using namespace ell::details;

  ASSERT_THROW(SignalHandler sh(SIGKILL,
                                [&](int) -> void
                                {
                                }),
               std::runtime_error);
}

// This test will quit the program.
TEST(test_signal_handler, desinstall)
{
  using namespace ell;
  using namespace ell::details;

  int count = 0;
  {
    SignalHandler sh(SIGINT, [&](int) -> void
                     {
                       count++;
                     });
    raise(SIGINT);
    ASSERT_EQ(1, count);
  }
  // Signal handler was un register when SignalHandler was destroyed.
  raise(SIGINT);
  ASSERT_TRUE(false);
}

int main(int ac, char **av)
{
  ell::initialize_logger();
  ::testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
