#include "upstream.hh"

namespace http
{
    UpstreamHost &Upstream::find_host()
    {
        if (config.method == UpstreamConfig::Method::FAILOVER)
            index = 0;
        auto start = index;
        do
        {
            auto &host = hosts[index];
            index = (index + 1) % hosts.size();
            if (config.method == UpstreamConfig::Method::ROUND_ROBIN)
                return host;
            if (host.up)
                return host;
        } while (start != index);

        throw RequestError(SERVICE_UNAVAILABLE);
    }

    std::map<std::string, shared_upstream> upstreams_map;
    std::vector<shared_upstream> upstreams;
} // namespace http
