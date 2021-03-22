/**
 * \file request/response.hh
 * \brief Response declaration.
 */

#pragma once

#include <filesystem>
#include <map>
#include <variant>

#include "request/message.hh"
#include "request/request.hh"
#include "request/types.hh"

namespace http
{
    class RecvResponseEW;
    /**
     * \struct Response
     * \brief Value object representing a response.
     */
    struct Response : public Message
    {
        explicit Response(const STATUS_CODE &);
        Response(const Request &req, const STATUS_CODE & = STATUS_CODE::OK);

        Response() = default;
        Response(const Response &s) = default;
        Response &operator=(const Response &) = default;
        Response(Response &&) = default;
        Response &operator=(Response &&) = default;
        ~Response() = default;

        std::string to_string() const;
        void set_date();
        static std::shared_ptr<Response>
        parse_response_line(const std::string &line);
        static bool parse(RecvResponseEW &ew);

        STATUS_CODE status;
        std::string status_message;

        bool is_file;
        size_t content_length;
    };

    using shared_res = std::shared_ptr<Response>;
} // namespace http
