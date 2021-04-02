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

    void VHostStaticFile::send_auto_index(shared_req req, shared_conn conn,
                                          fs::path path)
    {
        // TODO handle HEAD

        auto path_str = fs::relative(path, conf_.root).string();

        if (path_str == ".")
            path_str = "/";
        else if (path_str.find("./") == 0)
            path_str = path_str.substr(1);
        else if (path_str[0] != '/')
            path_str = '/' + path_str;

        std::string body = "<!DOCTYPE html>\n";
        body += "<html>\n";
        body += "<head>\n";
        body += "<meta charset=utf-8>\n";
        body += "<title>Index of " + path_str + "</title>\n";
        body += "</head>\n";
        body += "<body>\n";
        body += "<ul>\n";

        std::vector<std::string> filenames = { ".." };

        for (const auto &entry : fs::directory_iterator(path))
        {
            auto filename = entry.path().filename().string();
            filenames.push_back(filename);
        }

        for (const auto &filename : filenames)
        {
            auto subpath = path_str;
            if (subpath[subpath.size() - 1] != '/')
                subpath += '/';
            subpath += filename;

            body +=
                "<li><a href=\"" + subpath + "\">" + filename + "</a></li>\n";
        }

        body += "</ul>\n";
        body += "</body>\n";
        body += "</html>\n";

        auto res = std::make_shared<Response>();
        res->status = OK;
        res->status_message = statusCode(OK).second;
        res->set_date();

        if (req->headers.count("Connection") != 0
            && req->headers.at("Connection").find("close") != std::string::npos)
            res->headers["Connection"] = "close";
        else
            res->headers["Connection"] = "keep-alive";

        res->headers["Content-Length"] = std::to_string(body.size());
        res->body = body;

        SendResponseEW::start(conn, res);
    }

    void VHostStaticFile::respond(shared_req req, shared_conn conn)
    {
        if (!check_auth(req, conn, false))
            return;

        auto path = normalize_URI(req->target);

        if (fs::is_directory(path))
        {
            if (fs::is_regular_file(path / conf_.default_file))
                path /= conf_.default_file;
            else if (conf_.auto_index)
            {
                send_auto_index(req, conn, path);
                return;
            }
            else
                throw RequestError(NOT_FOUND);
        }
        else if (!fs::is_regular_file(path))
            throw RequestError(FORBIDDEN);

        req->target = path.string();

        // TODO make response take shared_req ?
        auto res = std::make_shared<Response>(*req, OK);

        SendResponseEW::start(conn, res);
    }
} // namespace http
