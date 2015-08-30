#include "details/task_builder.hpp"
#include <iostream>
#include <gtest/gtest.h>

using namespace ell::details;

int dummy0()
{
  return 42;
}

TEST(test_result_holder, construct_int_task)
{
  TaskBuilder builder;
  auto task = builder.build(dummy0);
}

int main(int ac, char **av)
{
  ::testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
