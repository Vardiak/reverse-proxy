#pragma once

#include <map>
#include <vector>

#include "config/config.hh"
#include "error/request-error.hh"
#include "request/types.hh"

namespace http
{
    struct UpstreamHost
    {
        UpstreamHostConfig config;
        bool up = true;
    };

    struct Upstream
    {
        Upstream(UpstreamConfig upstream_config)
            : config(upstream_config)
        {
            for (auto host_config : upstream_config.hosts)
            {
                UpstreamHost host;
                host.config = host_config;

                hosts.push_back(host);
            }
        }

        Upstream(VHostProxyPass proxy_pass)
        {
            config.method = UpstreamConfig::Method::ROUND_ROBIN;

            UpstreamHostConfig host_config;
            host_config.ip = proxy_pass.ip;
            host_config.port = proxy_pass.port;

            UpstreamHost host;
            host.config = host_config;

            hosts.push_back(host);
        }

        void health_check()
        {
            if (config.method == UpstreamConfig::Method::ROUND_ROBIN)
                return;

            // TODO: send request to each host
        }

        UpstreamHost &find_host();

        UpstreamConfig config;
        std::vector<UpstreamHost> hosts;
        unsigned index = 0;
    };

    using shared_upstream = std::shared_ptr<Upstream>;
    extern std::map<std::string, shared_upstream> upstreams_map;
    extern std::vector<shared_upstream> upstreams;
} // namespace http
