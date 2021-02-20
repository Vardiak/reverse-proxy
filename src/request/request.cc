#include "request.hh"

#include <regex>
#include <string>

#include "types.hh"

namespace http
{
    std::optional<std::shared_ptr<Request>>
    Request::parse(std::shared_ptr<Connection> connection_)
    {
        // connection_ = connection_;
        // return {};

        std::optional<std::shared_ptr<Request>> req;

        std::string &s = connection_->store;

        size_t last = 0;
        size_t next = 0;
        while ((next = s.find(http_crlf, last)) != std::string::npos)
        {
            std::string line = s.substr(last, next - last);
            if (!req)
                req = Request::parse_request_line(line);
            else
                Request::parse_request_header(line, req.value());
            last = next + 1;
        }
        return req;
    }

    std::optional<std::shared_ptr<Request>>
    Request::parse_request_line(std::string &line)
    {
        size_t i = line.find(' ');
        // 400 Bad Request
        if (i == std::string::npos)
            return std::nullopt;

        auto str = line.substr(0, i);
        // 501 Not Implemented
        if (str_method.find(str) == str_method.end())
            return std::nullopt;
        auto method = str_method[str];

        size_t j = line.find(' ', i + 1);
        // 400 Bad Request
        if (j == std::string::npos)
            return std::nullopt;

        std::string uri = line.substr(i + 1, j - i - 1);
        // 414 uri too long ?

        std::string http_version = line.substr(j + 1);
        const std::regex http_regex("HTTP/[0-9].[0-9]");
        // 400 Bad Request
        if (!std::regex_match(http_version, http_regex))
            return std::nullopt;

        // 505 HTTP version not supported
        if (http_version[5] != '1')
            return std::nullopt;

        return std::make_shared<Request>(method, uri, http_version.substr(5));
    }

    bool Request::parse_request_header(std::string &line,
                                       std::shared_ptr<Request> req)
    {
        size_t i = line.find(": ");

        if (i == std::string::npos)
            return false;

        std::string key = line.substr(0, i);

        std::string value = line.substr(i + 2);

        req->headers[key] = value;

        return true;
    }

} // namespace http
