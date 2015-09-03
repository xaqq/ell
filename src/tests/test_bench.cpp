#include <iostream>
#include "ell.hpp"

int incr()
{
  return 42;
}

int count_to_some_number()
{
  std::cout << "~~Tata~~" << std::endl;
  int count = 0;
  for (int i = 0; i < 10000000; i++)
  {
    count += ell::yield(incr);
  }
  std::cout << "Continuing tata..." << count << std::endl;
  return 1337;
}

int main()
{
  ell::initialize_logger();

  std::cout << "Size of TaskImpl: " << sizeof(ell::details::TaskImpl) << std::endl;
  ell::EventLoop l;
  auto task = l.call_soon(count_to_some_number);
  l.run_until_complete(task);

  std::cout << "Result = " << task->get_result() << std::endl;
  return 0;
}
