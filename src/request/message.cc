#include "message.hh"

#include <regex>
#include <string>

#include "error/request-error.hh"
#include "types.hh"

namespace http
{
    void Message::set_date()
    {
        char buf[1000];
        time_t now = time(0);
        struct tm tm = *gmtime(&now);
        strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &tm);
        headers["Date"] = buf;
    }

    void Message::parse_header(std::string &line)
    {
        const std::regex header_regex("^(\\S+):[ \t]*(.+?)[ \t]*$");

        std::smatch sm;
        if (!std::regex_search(line, sm, header_regex))
            throw RequestError(BAD_REQUEST);

        std::string key = sm[1];

        std::string value = sm[2];

        if (headers.count(key) > 0)
            throw RequestError(BAD_REQUEST);

        headers[key] = value;
    }
} // namespace http
