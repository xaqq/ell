#pragma once

#include <csignal>

#define ELL_ASSERT(cond, msg)                                                            \
  do                                                                                     \
  {                                                                                      \
    if (!(cond))                                                                         \
    {                                                                                    \
      assert(0 && msg);                                                                  \
      raise(SIGABRT);                                                                    \
    }                                                                                    \
  } while (0)
