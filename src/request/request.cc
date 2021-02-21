#include "request.hh"

#include <regex>
#include <string>

#include "error/request-error.hh"
#include "types.hh"

namespace http
{
    std::optional<std::shared_ptr<Request>>
    Request::parse(std::shared_ptr<Connection> conn)
    {
        std::optional<std::shared_ptr<Request>> &req = conn->req;
        std::string &s = conn->raw;
        size_t &last = conn->last;

        while (true)
        {
            // Delimitate line
            size_t next = s.find(http_crlf, last);

            // If not found, incomplete request
            if (next == std::string::npos)
                return std::nullopt;

            if (last == next) // If empty line
            {
                if (!req)
                {
                    // Skip line & continue parsing
                    last = next + 2;
                    continue;
                }

                std::shared_ptr<Request> r = req.value();
                if (r->headers.find("Content-Length") != r->headers.end())
                {
                    // Try to read body (check length) or incomplete request
                }

                req.reset();
                last = next + 2;
                return r;
            }

            std::string line = s.substr(last, next - last);

            if (!req)
                req = Request::parse_request_line(line);
            else
                Request::parse_request_header(line, req.value());

            last = next + 2;
        }
    }

    std::shared_ptr<Request> Request::parse_request_line(std::string &line)
    {
        size_t i = line.find(' ');
        if (i == std::string::npos)
            throw RequestError(BAD_REQUEST);

        auto str = line.substr(0, i);
        if (str_method.find(str) == str_method.end())
            throw RequestError(NOT_IMPLEMENTED);
        auto method = str_method[str];

        size_t j = line.find(' ', i + 1);
        if (j == std::string::npos)
            throw RequestError(BAD_REQUEST);

        std::string uri = line.substr(i + 1, j - i - 1);
        // 414 uri too long ?

        std::string http_version = line.substr(j + 1);
        const std::regex http_regex("HTTP/[0-9].[0-9]");
        if (!std::regex_match(http_version, http_regex))
            throw RequestError(BAD_REQUEST);

        if (http_version[5] != '1')
            throw RequestError(HTTP_VERSION_NOT_SUPPORTED);

        return std::make_shared<Request>(method, uri, http_version.substr(5));
    }

    void Request::parse_request_header(std::string &line,
                                       std::shared_ptr<Request> req)
    {
        size_t i = line.find(": ");

        if (i == std::string::npos)
            throw RequestError(BAD_REQUEST);

        std::string key = line.substr(0, i);

        std::string value = line.substr(i + 2);

        req->headers[key] = value;
    }

} // namespace http
