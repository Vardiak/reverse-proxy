#include <iostream>
#include <set>

#include "config/config.hh"
#include "error/init-error.hh"
#include "error/not-implemented.hh"
#include "events/event-loop.hh"
#include "events/listener.hh"
#include "events/register.hh"
#include "misc/addrinfo/addrinfo.hh"
#include "misc/readiness/readiness.hh"
#include "socket/default-socket.hh"
#include "vhost/dispatcher.hh"
#include "vhost/upstream.hh"
#include "vhost/vhost-factory.hh"

int main(int argc, char **argv)
{
    int i = 1;
    bool dry = false;
    if (argc == 3 && strcmp(argv[1], "-t") == 0)
    {
        i++;
        dry = true;
    }

    if (argc < 2 || (argc == 2 && argv[1][0] == '-'))
    {
        std::cerr << "Usage: ./spider [-t] config.json\n";
        return 1;
    }

    http::ServerConfig server_config;
    try
    {
        server_config = http::parse_configuration(argv[i]);

        for (auto &[key, value] : server_config.upstreams)
        {
            http::shared_upstream upstream =
                std::make_shared<http::Upstream>(value);

            http::upstreams.push_back(upstream);
            http::upstreams_map[key] = upstream;
        }

        for (auto vhost_config : server_config.vhosts)
        {
            auto vhost = http::VHostFactory::Create(vhost_config);

            http::dispatcher.add_vhost(vhost);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (dry)
        return 0;

    misc::announce_spider_readiness(argv[0]);

    http::event_register.launch_loop();

    return 0;
}
