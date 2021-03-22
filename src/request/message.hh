/**
 * \file request/Message.hh
 * \brief Message declaration.
 */

#pragma once

#include <filesystem>
#include <map>
#include <variant>

namespace http
{
    /**
     * \struct Message
     * \brief Value object representing a Message.
     */
    struct Message
    {
        Message() = default;
        Message(const Message &s) = default;
        Message &operator=(const Message &) = default;
        Message(Message &&) = default;
        Message &operator=(Message &&) = default;
        ~Message() = default;

        virtual std::string to_string() const = 0;

        void parse_header(std::string &line);

        std::map<std::string, std::string> headers;

        std::string body;
    };
} // namespace http
