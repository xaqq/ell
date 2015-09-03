#pragma once

#include "spdlog/spdlog.h"
#include "details/ell_assert.hpp"

#ifdef ELL_ENABLE_TRACE
#define ELL_TRACE(msg, ...)                                                              \
  {                                                                                      \
    auto logger = spdlog::get("ell_console");                                            \
    if (logger)                                                                          \
    {                                                                                    \
      SPDLOG_TRACE(logger, msg, ##__VA_ARGS__);                                          \
    }                                                                                    \
  }
#else
#define ELL_TRACE(...)
#endif

#ifdef ELL_ENABLE_DEBUG
#define ELL_DEBUG(msg, ...)                                                              \
  {                                                                                      \
    auto logger = spdlog::get("ell_console");                                            \
    if (logger)                                                                          \
    {                                                                                    \
      SPDLOG_DEBUG(logger, msg, ##__VA_ARGS__);                                          \
    }                                                                                    \
  }

#else
#define ELL_DEBUG(...)
#endif