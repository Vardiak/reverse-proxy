#include "vhost-reverse-proxy.hh"

#include <filesystem>
#include <iostream>
#include <regex>
#include <sstream>
#include <time.h>

#include "error/init-error.hh"
#include "error/request-error.hh"
#include "events/register.hh"
#include "events/send-request.hh"
#include "events/send-response.hh"

namespace http
{
    VHostReverseProxy::VHostReverseProxy(const VHostConfig &config)
        : VHost(config)
    {
        if (config.proxy_pass->upstream != "")
        {
            if (upstreams_map.count(config.proxy_pass->upstream) == 0)
                throw http::InitializationError("No upstream to link");
            upstream = upstreams_map[config.proxy_pass->upstream];
        }
        else
        {
            upstream = std::make_shared<Upstream>(*config.proxy_pass);
            upstreams.push_back(upstream);
        }
    }

    shared_socket
    VHostReverseProxy::connect_host(const UpstreamHostConfig &config)
    {
        auto hint = misc::AddrInfoHint()
                        .family(AF_UNSPEC)
                        .socktype(SOCK_STREAM)
                        .flags(AI_PASSIVE);

        auto addrinfos = misc::getaddrinfo(
            config.ip.c_str(), std::to_string(config.port).c_str(), hint);

        for (auto a : addrinfos)
        {
            try
            {
                shared_socket socket = std::make_shared<DefaultSocket>(
                    a.ai_family, a.ai_socktype, a.ai_protocol);

                socket->connect(a.ai_addr, a.ai_addrlen);

                fcntl(socket->fd_get()->fd_, F_SETFL, O_NONBLOCK);

                return socket;
            }
            catch (const std::system_error &e)
            {
                std::cerr << e.what() << std::endl;
                continue;
            }
        }

        return nullptr;
    }

    static std::vector<std::string> split(std::string str, char delimiter)
    {
        std::stringstream stream(str);
        std::vector<std::string> res;
        std::string s;
        while (std::getline(stream, s, delimiter))
            res.push_back(s);
        return res;
    }

    void forwarded_transition(shared_req req)
    {
        if (req->headers.count("Forwarded") != 0)
            return;

        bool for_ip = req->headers.count("X-Forwarded-For") != 0;
        bool host = req->headers.count("X-Forwarded-Host") != 0;
        bool proto = req->headers.count("X-Forwarded-Proto") != 0;

        if ((for_ip && host) || (host && proto) || (proto && for_ip)
            || (!proto && !for_ip && !host))
            return;

        std::string prop = for_ip ? "For" : host ? "Host" : "Proto";
        std::string prop_lower = for_ip ? "for=" : host ? "host=" : "proto=";

        std::vector<std::string> forwarded =
            split(req->headers["X-Forwarded-" + prop], ',');
        std::string res;
        for (size_t i = 0; i < forwarded.size(); i++)
        {
            if (forwarded[i][0] == ' ')
                forwarded[i] = forwarded[i].substr(1);
            res += prop_lower;
            if (for_ip && forwarded[i].find(':') != std::string::npos)
                res += "\"[" + forwarded[i] + "]\"";
            else
                res += forwarded[i];
            if (i != forwarded.size() - 1)
                res += ", ";
        }

        req->headers.erase("X-Forwarded-" + prop);
        req->headers["Forwarded"] = res;
    }

    bool VHostReverseProxy::remove_headers(shared_req &req)
    {
        bool res = true;
        const std::vector<std::string> hop_headers = {
            "Keep-Alive",        "Transfer-Encoding", "TE",
            "Trailer",           "Upgrade",           "Proxy-Authorization",
            "Proxy-Authenticate"
        };
        for (auto &header : hop_headers)
            req->headers.erase(header);

        if (req->headers.count("Connection") == 0)
            return true;

        const std::vector<std::string> connection_headers =
            split(req->headers["Connection"], ',');
        for (auto header : connection_headers)
        {
            if (header[0] == ' ')
                header = header.substr(1);
            req->headers.erase(header);
            if (header == "close")
                res = false;
        }
        return res;
    }

    void VHostReverseProxy::respond(shared_req req,
                                    std::shared_ptr<Connection> conn)
    {
        if (!check_auth(req, conn, true))
            return;

        auto host = upstream->find_host();
        auto backend_sock = connect_host(host.config);
        if (!backend_sock)
            throw RequestError(BAD_GATEWAY);

        bool keep_alive = remove_headers(req);

        req->headers["Connection"] = "close";

        forwarded_transition(req);

        // Handle forward headers
        std::string forwarded = "for=" + conn->client_ip
            + ";host=" + req->headers["Host"]
            + ";proto=" + (conf_.ssl_cert.empty() ? "http" : "https");
        if (req->headers.count("Forwarded"))
            req->headers["Forwarded"] += ", " + forwarded;
        else
            req->headers["Forwarded"] = forwarded;

        req->headers["Host"] =
            host.config.ip + ":" + std::to_string(host.config.port);

        for (auto &[key, value] : conf_.proxy_pass->proxy_set_header)
            req->headers[key] = value;

        for (auto &key : conf_.proxy_pass->proxy_remove_header)
            req->headers.erase(key);

        SendRequestEW::start(
            backend_sock, req, conf_.proxy_pass->timeout,
            [this, conn, keep_alive](shared_res res) {
                for (auto &[key, value] : this->conf_.proxy_pass->set_header)
                    res->headers[key] = value;

                for (auto &key : this->conf_.proxy_pass->remove_header)
                    res->headers.erase(key);

                if (res->headers.count("Content-Length") == 0)
                    res->headers["Content-Length"] = "0";

                res->headers["Connection"] =
                    keep_alive ? "keep-alive" : "close";

                SendResponseEW::start(conn, res);
            });
    }
} // namespace http
