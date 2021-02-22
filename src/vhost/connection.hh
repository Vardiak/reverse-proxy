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
        Connection(shared_socket sock, std::string ip, uint16_t port)
            : sock_(sock)
            , ip_(ip)
            , port_(port)
        {}
        Connection(const Connection &con) = default;
        Connection &operator=(const Connection &) = default;
        Connection(Connection &&) = default;
        Connection &operator=(Connection &&) = default;
        ~Connection() = default;

        shared_socket sock_;

        // Server IP & port
        std::string ip_;
        uint16_t port_;

        std::optional<std::shared_ptr<Request>> req;
        std::string raw;
        size_t last = 0;
    };

    using shared_conn = std::shared_ptr<Connection>;
} // namespace http
