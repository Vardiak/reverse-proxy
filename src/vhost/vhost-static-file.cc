#include "vhost-static-file.hh"

#include <filesystem>
#include <iostream>
#include <time.h>

#include "error/request-error.hh"
#include "events/register.hh"
#include "events/send-response.hh"

namespace fs = std::filesystem;

namespace http
{
    VHostStaticFile::VHostStaticFile(const VHostConfig &config)
        : VHost(config)
    {}

    fs::path VHostStaticFile::normalizeURI(std::string uri)
    {
        auto base = fs::canonical(fs::absolute(conf_.root));

        auto res = base;
        if (!fs::exists(res += uri))
            throw RequestError(NOT_FOUND);

        res = fs::canonical(res);

        std::cout << "Path: " << res.string() << std::endl;

        if (res.string().find(base.string()) != 0)
            throw RequestError(FORBIDDEN);

        return res;
    }

    void VHostStaticFile::respond(Request &req,
                                  std::shared_ptr<Connection> conn)
    {
        auto path = normalizeURI(req.target);

        if (fs::is_directory(path))
        {
            path /= conf_.default_file;

            if (!fs::exists(path))
                throw RequestError(NOT_FOUND);
        }
        else if (!fs::is_regular_file(path))
            throw RequestError(FORBIDDEN);

		req.target = path.string();

        auto res = std::make_shared<Response>(req, OK);
        
        shared_socket sock = conn->sock_;
        event_register
            .register_event<SendResponseEW, shared_socket, shared_res>(
                std::move(sock), std::move(res));
    }
} // namespace http
