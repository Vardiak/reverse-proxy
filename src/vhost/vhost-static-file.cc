#include "vhost-static-file.hh"

#include <filesystem>
#include <iostream>
#include <time.h>

#include "error/request-error.hh"
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
        std::string path = conf_.root + req.uri;

        if (!std::filesystem::exists(path))
            throw RequestError(NOT_FOUND);

        if (std::filesystem::is_directory(path))
        {
            if (path.back() != '/')
                path += '/';
            path += conf_.default_file;
        }
        else if (!std::filesystem::is_regular_file(path))
            throw RequestError(FORBIDDEN);

        auto res = std::make_shared<Response>(req, OK);

        res->body = path;

        res->content_length =
            res->is_file ? std::filesystem::file_size(path) : 0;

        res->headers["Connection"] = "close";

        res->headers["Content-Length"] = std::to_string(res->content_length);

        shared_socket sock = conn->sock_;

        event_register
            .register_event<SendResponseEW, shared_socket, shared_res>(
                std::move(sock), std::move(res));
    }
} // namespace http
