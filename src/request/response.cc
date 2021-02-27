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
        , body("")
        , content_length(body.length())
    {
        headers["Connection"] = "close";
        headers["Content-Length"] = std::to_string(content_length);

        set_date();
    }

    Response::Response(const Request &req, const STATUS_CODE &s)
        : status(s)
        , is_file(true)
    {
        headers["Connection"] = "close";
        set_date();

        if (req.method != METHOD::HEAD)
            body = req.target;
        else
        {
            body = "";
            is_file = false;
        }

        content_length = is_file ? std::filesystem::file_size(req.target) : 0;
        headers["Content-Length"] = std::to_string(content_length);
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
