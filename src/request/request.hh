/**
 * \file request/request.hh
 * \brief Request declaration.
 */

#pragma once

#include <memory>
#include <optional>

#include "request/types.hh"
#include "vhost/connection.hh"

namespace http
{
    class Connection;

    /**
     * \struct Request
     * \brief Value object representing a request.
     */
    struct Request
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

        static std::optional<std::shared_ptr<Request>>
            parse(std::shared_ptr<Connection>);

        static std::shared_ptr<Request> parse_request_line(std::string &line);

        static void parse_request_header(std::string &line,
                                         std::shared_ptr<Request> req);

        METHOD method;
        std::string target;
        std::string http_version;

        std::map<std::string, std::string> headers;

        // FIXME: Add members to store the information relative to a request.
    };
} // namespace http
