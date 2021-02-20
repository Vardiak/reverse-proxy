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
    /**
     * \struct Request
     * \brief Value object representing a request.
     */
    struct Request
    {
        Request(METHOD m, std::string u, std::string v)
            : method(m)
            , uri(u)
            , http_version(v)
        {}
        Request(const Request &) = default;
        Request &operator=(const Request &) = default;
        Request(Request &&) = default;
        Request &operator=(Request &&) = default;
        ~Request() = default;

        static std::optional<std::shared_ptr<Request>>
            parse(std::shared_ptr<Connection>);

        static std::optional<std::shared_ptr<Request>>
        parse_request_line(std::string &line);

        static bool parse_request_header(std::string &line,
                                         std::shared_ptr<Request> req);

        METHOD method;
        std::string uri;
        std::string http_version;

        std::map<std::string, std::string> headers;

        // FIXME: Add members to store the information relative to a request.
    };
} // namespace http
