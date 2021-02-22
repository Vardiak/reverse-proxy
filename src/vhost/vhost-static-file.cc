#include "vhost-static-file.hh"

#include <filesystem>
#include <iostream>
#include <time.h>

#include "events/register.hh"
#include "events/send-response.hh"

namespace http
{
    VHostStaticFile::VHostStaticFile(const VHostConfig &config)
        : VHost(config)
    {}

    void VHostStaticFile::respond(Request &req,
                                  std::shared_ptr<Connection> conn)
    {
        // Check directory, 404 etc.

        auto res = std::make_shared<Response>(req, OK);
        res->body = conf_.root + req.uri;

        res->headers["Connection"] = "close";

        char buf[1000];
        time_t now = time(0);
        struct tm tm = *gmtime(&now);
        strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
        res->headers["Date"] = buf;

        res->headers["Content-Length"] = std::to_string(
            res->is_file ? std::filesystem::file_size(res->body) : 0);

        shared_socket sock = conn->sock_;

        std::cout << "Before sending response\n";

        event_register
            .register_event<SendResponseEW, shared_socket, shared_res>(
                std::move(sock), std::move(res));

        std::cout << "After sending response\n";

        // Fri, 07 Feb 2020 21:51:28 GMT
    }
} // namespace http
