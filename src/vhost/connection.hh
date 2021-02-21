/**
 * \file vhost/connection.hh
 * \brief Connection declaration.
 */

#pragma once

#include <memory>
#include <string>

#include "request/request.hh"
#include "request/types.hh"
#include "socket/socket.hh"

namespace http
{
    class Request;

    /**
     * \struct Connection
     * \brief Value object representing a connection.
     *
     * We need to keep track of the state of each request while it has not
     * been fully processed.
     */
    struct Connection
    {
        Connection() = default;
        Connection(const Connection &con) = default;
        Connection &operator=(const Connection &) = default;
        Connection(Connection &&) = default;
        Connection &operator=(Connection &&) = default;
        ~Connection() = default;

        /* FIXME: Add members to store the information relative to the
        ** connection.
        */

        std::optional<std::shared_ptr<Request>> req;
        std::string raw;
        size_t last = 0;

        std::string ip_;
        uint16_t port_;
    };
} // namespace http
