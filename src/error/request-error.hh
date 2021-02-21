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

        STATUS_CODE status;
    };
} // namespace http
