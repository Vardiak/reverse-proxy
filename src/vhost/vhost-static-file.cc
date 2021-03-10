#include "vhost-static-file.hh"

#include <filesystem>
#include <iostream>
#include <regex>
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

    std::string VHostStaticFile::uri_decode(std::string &uri)
    {
        std::string res;
        for (size_t i = 0; i < uri.length(); i++)
        {
            if (uri[i] == '%')
            {
                char c = std::strtol(uri.substr(i + 1, 2).c_str(), nullptr, 16);
                res += c;
                i = i + 2;
            }
            else
                res += uri[i];
        }
        return res;
    }

    fs::path VHostStaticFile::normalize_URI(std::string uri)
    {
        const std::regex uri_regex("^([^?#]*)(\\?[^#]*)?(#(.*))?");

        std::smatch sm;
        if (!std::regex_search(uri, sm, uri_regex))
            throw RequestError(BAD_REQUEST);
        uri = sm[1];

        try
        {
            uri = uri_decode(uri);
        }
        catch (const std::exception &e)
        {
            throw RequestError(BAD_REQUEST);
        }

        auto base = fs::canonical(fs::absolute(conf_.root));

        auto res = base;
        if (!fs::exists(res += uri))
            throw RequestError(NOT_FOUND);

        res = fs::canonical(res);

        if (res.string().find(base.string()) != 0)
            throw RequestError(FORBIDDEN);

        return res;
    }

    void VHostStaticFile::respond(Request &req,
                                  std::shared_ptr<Connection> conn)
    {
        if (!check_auth(req, conn))
            return;

        auto path = normalize_URI(req.target);

        if (fs::is_directory(path))
        {
            path /= conf_.default_file;

            if (!fs::is_regular_file(path))
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
