#include <iostream>
#include "ell.hpp"

int tata()
{
  std::cout << "~~Tata~~" << std::endl;
  for (int i = 0; i < 2000; i++)
  {
    ell::yield();
  }
  std::cout << "Continuing tata..." << std::endl;
  return 1337;
}

int toto()
{
  std::cout << "~~Toto~~" << std::endl;
  ell::yield();
  std::cout << "~~Titi~~" << std::endl;

  std::cout << "Return value from Tata: " << ell::yield(tata) << std::endl;
  return 42;
}

int main()
{
  // ell::initialize_logger();
  ell::EventLoop l;
  auto task = l.call_soon(toto);
  l.run_until_complete(task);

  std::cout << "Result = " << task->get_result() << std::endl;
  return 0;
}