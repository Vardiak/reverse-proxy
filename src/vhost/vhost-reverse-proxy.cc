#include "vhost-reverse-proxy.hh"

#include <filesystem>
#include <iostream>
#include <regex>
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

                fcntl(socket->fd_get()->fd_, F_SETFL, O_NONBLOCK);

                socket->connect(a.ai_addr, a.ai_addrlen);

                return socket;
            }
            catch (const std::system_error &e)
            {
                continue;
            }
        }

        throw RequestError(BAD_GATEWAY);
    }

    void VHostReverseProxy::respond(shared_req req,
                                    std::shared_ptr<Connection> conn)
    {
        if (!check_auth(req, conn))
            return;

        for (auto &[key, value] : conf_.proxy_pass->proxy_set_header)
        {
            req->headers[key] = value;
        }

        for (auto &key : conf_.proxy_pass->proxy_remove_header)
        {
            if (req->headers.count(key))
                req->headers.erase(key);
        }

        // Handle forward headers

        auto host = upstream->find_host();

        auto backend_sock = connect_host(host.config);

        SendRequestEW::start(backend_sock, req, [conn](shared_res) {
            // SendReponseEW.start(conn, res);
        });
    }
} // namespace http
