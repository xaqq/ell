#pragma once

#include "ell_fwd.hpp"
#include "details/ell_log.hpp"
#include "details/ell_assert.hpp"

namespace ell
{
}

#include "task.hpp"
#include "details/default_event_loop.hpp"

namespace ell
{
  /**
   * Initialize the logging system.
   */
  void initialize_logger()
  {
    auto console = spdlog::stdout_logger_mt("ell_console");
    ELL_ASSERT(console, "Cannot create logger.");
    console->set_level(spdlog::level::trace);
    ELL_DEBUG("Hello {}!", 1);
    ELL_DEBUG("World");
  }

  /**
  * Simply yield, being nice and giving other a chance to run!
  */
  void yield()
  {
    details::get_current_event_loop()->current_task_suspend();
  }

  template <typename Duration>
  void sleep(const Duration &duration)
  {
    details::get_current_event_loop()->current_task_sleep(duration);
  }

  /**
   * Yield to an other callable, waiting for this callable
   * to return in order to continue.
   */
  template <typename Callable>
  auto yield(const Callable &callable) -> decltype(callable())
  {
    auto loop = details::get_current_event_loop();
    return loop->yield(callable);
  }
}