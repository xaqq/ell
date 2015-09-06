#pragma once

namespace ell
{
  namespace details
  {
    /**
     * Wrap a signal handler.
     *
     * Install the signal handler in the constructor, and detach
     * it in the destructor.
     */
    class SignalHandler
    {
    public:
      template <typename T>
      SignalHandler(int signum, const T &callable)
          : signum_(signum)
      {
        auto handler = [=](int sig)
        {
          callable(sig);
        };
        set_handler_for(signum, handler);
        set_system_handler(signum, &invoke);
      }

      ~SignalHandler()
      {
        try
        {
          set_system_handler(signum_, SIG_DFL);
        }
        catch (const std::runtime_error &e)
        {
          ELL_ERROR("Cannot unregister signal handler for signal {}", signum_);
        }
      }

    private:
      void set_system_handler(int signum, void (*handler)(int))
      {
        struct sigaction act;
        memset(&act, '\0', sizeof(act));
        act.sa_handler = handler;
        if (sigaction(signum, &act, NULL) < 0)
        {
          std::array<char, 256> buffer{0};
          char *err = strerror_r(errno, &buffer[0], buffer.size());
          ELL_ERROR("Error installing signal: {}", err);

          throw std::runtime_error(err);
        }
      }

      using UserSignalHandler = std::function<void(int)>;

      static UserSignalHandler &
      set_handler_for(int signum, const UserSignalHandler &new_handler, bool set = true)
      {
        static thread_local std::function<void(int)> handler[50];
        ELL_ASSERT(signum > 0 && signum < 50, "Signum out of bound.");
        if (set)
          handler[signum] = new_handler;
        return handler[signum];
      }

      static UserSignalHandler &get_handler_for(int signum)
      {
        UserSignalHandler h = nullptr;
        return set_handler_for(signum, h, false);
      }

      static void invoke(int signum)
      {
        auto handler = get_handler_for(signum);
        handler(signum);
      }

      int signum_;
    };
  }
}
