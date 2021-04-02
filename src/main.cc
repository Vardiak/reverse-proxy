#include <cstdlib>
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

static void timeout_cb(EV_P_ ev_timer *, int)
{
    for (auto upstream : http::upstreams)
    {
        upstream->health_check();
    }
}

int main(int argc, char **argv)
{
    int i = 1;
    bool dry = false;
    ev_timer timeout_watcher;
    signal(SIGPIPE, SIG_IGN);

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

    try
    {
        http::parse_configuration(argv[i]);

        for (auto &[key, value] : http::server_config.upstreams)
        {
            http::shared_upstream upstream =
                std::make_shared<http::Upstream>(value);

            http::upstreams.push_back(upstream);
            http::upstreams_map[key] = upstream;
        }

        for (auto vhost_config : http::server_config.vhosts)
        {
            auto vhost = http::VHostFactory::Create(vhost_config);

            http::dispatcher.add_vhost(vhost);
        }

        if (!http::upstreams.empty())
        {
            ev_timer_init(&timeout_watcher, timeout_cb, 0.,
                          atoi(argv[argc - 1]));
            ev_timer_start(http::event_register.loop_.loop, &timeout_watcher);
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
