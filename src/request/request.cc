#include "request.hh"

#include <regex>
#include <string>

#include "error/request-error.hh"
#include "types.hh"
#include "vhost/connection.hh"

namespace http
{
    std::shared_ptr<Request> Request::parse(std::shared_ptr<Connection> conn)
    {
        std::shared_ptr<Request> &req = conn->req;
        std::string &s = conn->raw;
        size_t &last = conn->last;

        while (true)
        {
            // Delimitate line
            size_t next = s.find(http_crlf, last);

            // If not found, incomplete request
            if (next == std::string::npos)
                return nullptr;

            if (last == next) // If empty line
            {
                if (!req)
                {
                    // Skip line & continue parsing
                    last = next + 2;
                    continue;
                }

                std::shared_ptr<Request> r = req;
                if (r->headers.count("Content-Length") != 0)
                {
                    try
                    {
                        int size = std::stoi(r->headers["Content-Length"]);

                        if (last + 2 + size > s.size())
                            return nullptr;

                        r->body = s.substr(last + 2, size);
                    }
                    catch (const std::invalid_argument &e)
                    {
                        throw RequestError(BAD_REQUEST);
                    }
                }

                const std::regex host_regex("^[\\w\\.-]+(:[0-9]+)?$");

                if (r->headers.count("Host") == 0
                    || !std::regex_match(r->headers["Host"], host_regex))
                    throw RequestError(BAD_REQUEST);

                req.reset();
                return r;
            }

            std::string line = s.substr(last, next - last);

            if (!req)
                req = Request::parse_request_line(line);
            else
                req->parse_header(line);

            last = next + 2;
        }
    }

    std::shared_ptr<Request> Request::parse_request_line(std::string &line)
    {
        size_t i = line.find(' ');
        if (i == std::string::npos)
            throw RequestError(BAD_REQUEST);

        auto method = line.substr(0, i);

        size_t j = line.find(' ', i + 1);
        if (j == std::string::npos)
            throw RequestError(BAD_REQUEST);

        std::string target = line.substr(i + 1, j - i - 1);
        std::smatch sm;
        const std::regex url("(?:https?://[a-zA-Z0-9.]+(?::[0-9]*)?)?(/.*)");
        if (!std::regex_search(target, sm, url))
            throw RequestError(BAD_REQUEST);
        target = sm[1];

        std::string http_version = line.substr(j + 1);
        const std::regex http_regex("HTTP/[0-9]\\.[0-9]");
        if (!std::regex_match(http_version, http_regex))
            throw RequestError(BAD_REQUEST);

        if (http_version[5] != '1' || (http_version[7] - '0') > 1)
            throw RequestError(HTTP_VERSION_NOT_SUPPORTED);

        if (http_version[7] != '1')
            throw RequestError(UPGRADE_REQUIRED);

        return std::make_shared<Request>(method, target,
                                         http_version.substr(5));
    }

    std::string Request::to_string() const
    {
        std::string raw;

        raw += method;
        raw += ' ';
        raw += target;
        raw += ' ';
        raw += "HTTP/";
        raw += http_version;
        raw += http_crlf;

        for (auto const &[key, value] : headers)
            raw += key + ": " + value + "\r\n";

        raw += "\r\n";

        raw += body;

        return raw;
    }

} // namespace http
