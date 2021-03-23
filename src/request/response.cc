#include "response.hh"

#include <istream>
#include <regex>
#include <sstream>
#include <string>

#include "events/recv-response.hh"
#include "types.hh"

namespace http
{
    Response::Response(const STATUS_CODE &s)
        : status(s)
        , status_message(statusCode(status).second)
        , is_file(false)
        , content_length(body.length())
    {
        headers["Connection"] = "close";
        headers["Content-Length"] = std::to_string(content_length);

        set_date();
    }

    Response::Response(const Request &req, const STATUS_CODE &s)
        : status(s)
        , status_message(statusCode(status).second)
        , is_file(true)
    {
        if (req.headers.count("Connection") != 0
            && req.headers.at("Connection").find("close") != std::string::npos)
            headers["Connection"] = "close";
        else
            headers["Connection"] = "keep-alive";

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

    std::string Response::to_string() const
    {
        std::string raw;

        raw += "HTTP/1.1 ";
        raw += std::to_string(status);
        raw += " ";
        raw += status_message;
        raw += http_crlf;

        for (auto const &[key, value] : headers)
            raw += key + ": " + value + "\r\n";

        raw += "\r\n";

        if (!is_file)
            raw += body;

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

    shared_res Response::parse_response_line(const std::string &line)
    {
        shared_res res = std::make_shared<Response>();

        const std::regex reg("^HTTP\\/[0-9]\\.[0-9] ([0-9]+) (.+)$");
        std::smatch sm;

        std::regex_search(line, sm, reg);
        res->status = static_cast<STATUS_CODE>(std::stoi(sm[1]));
        res->status_message = sm[2];
        return res;
    }

    bool Response::parse(RecvResponseEW &ew)
    {
        std::shared_ptr<Response> &res = ew.res;
        std::string &s = ew.raw;
        size_t &last = ew.last;

        while (true)
        {
            // Delimitate line
            size_t next = s.find(http_crlf, last);

            // If not found, incomplete Response
            if (next == std::string::npos)
                return false;

            if (last == next) // If empty line
            {
                if (!res)
                {
                    // Skip line & continue parsing
                    last = next + 2;
                    continue;
                }

                if (res->headers.count("Content-Length") != 0)
                {
                    int size = std::stoi(res->headers["Content-Length"]);

                    if (last + 2 + size > s.size())
                        return false;

                    res->body = s.substr(last + 2, size);
                }

                return true;
            }

            std::string line = s.substr(last, next - last);

            if (!res)
                res = Response::parse_response_line(line);
            else
                res->parse_header(line);

            last = next + 2;
        }
    }
} // namespace http
