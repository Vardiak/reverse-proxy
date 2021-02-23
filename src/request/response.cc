#include "request/response.hh"

#include <istream>
#include <sstream>
#include <string>

#include "types.hh"

namespace http
{
    Response::Response(const STATUS_CODE &s)
        : status(s)
        , is_file(false)
        , body(std::string(statusCode(status).second) + "\n")
        , content_length(body.length())
    {
        headers["Connection"] = "close";
        headers["Content-Length"] = std::to_string(content_length);

        set_date();
    }

    Response::Response(const Request &, const STATUS_CODE &s)
        : status(s)
        , is_file(true)
    {
        set_date();
    }

    std::string Response::to_string()
    {
        std::string raw;

        raw += "HTTP/1.1 ";
        raw += std::to_string(status);
        raw += " ";
        raw += statusCode(status).second;
        raw += http_crlf;

        for (auto const &[key, value] : headers)
            raw += key + ": " + value + "\r\n";

        raw += "\r\n";

        if (!is_file)
        {
            raw += body;
        }

        return raw;
    }

    void Response::set_date()
    {
        char buf[1000];
        time_t now = time(0);
        struct tm tm = *gmtime(&now);
        strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &tm);
        headers["Date"] = buf;
    }
} // namespace http
