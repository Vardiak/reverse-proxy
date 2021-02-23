/**
 * \file request/response.hh
 * \brief Response declaration.
 */

#pragma once

#include <filesystem>
#include <map>
#include <variant>

#include "request/request.hh"
#include "request/types.hh"

namespace http
{
    /**
     * \struct Response
     * \brief Value object representing a response.
     */
    struct Response
    {
        explicit Response(const STATUS_CODE &);
        Response(const Request &, const STATUS_CODE & = STATUS_CODE::OK);

        Response() = default;
        Response(const Response &s) = default;
        Response &operator=(const Response &) = default;
        Response(Response &&) = default;
        Response &operator=(Response &&) = default;
        ~Response() = default;

        std::string to_string();
        void set_date();

        STATUS_CODE status;

        std::map<std::string, std::string> headers;

        bool is_file;
        std::string body;
        size_t content_length;
    };

    using shared_res = std::shared_ptr<Response>;
} // namespace http
