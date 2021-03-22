/**
 * \file request/request.hh
 * \brief Request declaration.
 */

#pragma once

#include <memory>
#include <optional>

#include "request/message.hh"
#include "request/types.hh"

namespace http
{
    class Connection;

    /**
     * \struct Request
     * \brief Value object representing a request.
     */
    struct Request : public Message
    {
        Request(METHOD m, std::string t, std::string v)
            : method(m)
            , target(t)
            , http_version(v)
        {}
        Request(const Request &) = default;
        Request &operator=(const Request &) = default;
        Request(Request &&) = default;
        Request &operator=(Request &&) = default;
        ~Request() = default;

        static std::shared_ptr<Request> parse(std::shared_ptr<Connection>);

        static std::shared_ptr<Request> parse_request_line(std::string &line);
        std::string to_string() const;

        METHOD method;
        std::string target;
        std::string http_version;
    };

    using shared_req = std::shared_ptr<Request>;

} // namespace http
