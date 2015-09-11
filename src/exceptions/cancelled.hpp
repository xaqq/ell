#pragma once

#include <exception>

namespace ell
{
    namespace ex
    {
        /**
         * Exception thrown when a Task is cancelled.
         */
        class Cancelled : public std::exception
        {

        };
    }
}