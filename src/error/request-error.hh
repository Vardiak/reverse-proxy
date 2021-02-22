#pragma once

#include <stdexcept>
#include <string>

#include "request/types.hh"

namespace http
{
    struct RequestError : public std::exception
    {
        explicit RequestError(STATUS_CODE s)
            : status(s)
        {}
        virtual ~RequestError() = default;

        virtual const char *what() const noexcept
        {
            return statusCode(status).second;
        }

        STATUS_CODE status;
    };
} // namespace http
