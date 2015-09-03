#pragma once

#include "spdlog/spdlog.h"
#include "details/ell_assert.hpp"

#define ELL_TRACE(msg, ...)                                                              \
  {                                                                                      \
    auto logger = spdlog::get("ell_console");                                            \
    ELL_ASSERT(logger, "Cannot retrieve spdlog logger object.");                         \
    SPDLOG_TRACE(logger, msg, ##__VA_ARGS__);                                            \
  }

#define ELL_DEBUG(msg, ...)                                                              \
  {                                                                                      \
    auto logger = spdlog::get("ell_console");                                            \
    ELL_ASSERT(logger, "Cannot retrieve spdlog logger object.");                         \
    SPDLOG_DEBUG(logger, msg, ##__VA_ARGS__);                                            \
  }
