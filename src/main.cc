#include <iostream>
#include <set>

#include "config/config.hh"
#include "error/not-implemented.hh"
#include "events/event-loop.hh"
#include "events/listener.hh"
#include "events/register.hh"
#include "misc/addrinfo/addrinfo.hh"
#include "misc/readiness/readiness.hh"
#include "socket/default-socket.hh"
#include "vhost/dispatcher.hh"
#include "vhost/vhost-factory.hh"

http::DefaultSocket prepare_socket(http::VHostConfig config);
http::DefaultSocket create_and_bind(const misc::AddrInfo &addrinfos);

http::DefaultSocket create_and_bind(const misc::AddrInfo &addrinfos)
{
    for (auto a : addrinfos)
    {
        try
        {
            http::DefaultSocket socket(a.ai_family, a.ai_socktype,
                                       a.ai_protocol);

            socket.bind(a.ai_addr, a.ai_addrlen);
            socket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1);

            return socket;
        }
        catch (const std::system_error &e)
        {
            continue;
        }
    }

    throw std::runtime_error("Couldn't bind IP and port");
}

http::DefaultSocket prepare_socket(http::VHostConfig config)
{
    auto hint = misc::AddrInfoHint()
                    .family(AF_UNSPEC)
                    .socktype(SOCK_STREAM)
                    .flags(AI_PASSIVE);

    auto addrinfos = misc::getaddrinfo(
        config.ip.c_str(), std::to_string(config.port).c_str(), hint);

    auto socket = create_and_bind(addrinfos);
    socket.listen(1);
    return socket;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: ./spider [path to server config]\n";
        return 1;
    }

    auto server_config = http::parse_configuration(argv[1]);

    std::set<std::string> listened;

    for (auto vhost_config : server_config.vhosts)
    {
        std::string key =
            vhost_config.ip + "+" + std::to_string(vhost_config.port);
        if (listened.count(key) == 0)
        {
            auto socket = std::make_shared<http::DefaultSocket>(
                prepare_socket(vhost_config));

            std::string ip = vhost_config.ip;
            uint16_t port = vhost_config.port;

            http::event_register
                .register_event<http::ListenerEW, http::shared_socket,
                                std::string, std::uint16_t>(
                    socket, std::move(ip), std::move(port));

            listened.insert(key);
        }

        auto vhost = http::VHostFactory::Create(vhost_config);

        http::dispatcher.add_vhost(vhost);
    }

    misc::announce_spider_readiness(argv[0]);

    http::event_register.launch_loop();

    return 0;
}
