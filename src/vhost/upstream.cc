#include "upstream.hh"

#include "events/send-request.hh"
#include "request/request.hh"
#include "request/response.hh"
#include "vhost/vhost-reverse-proxy.hh"

namespace http
{
    void Upstream::health_check()
    {
        if (config.method == UpstreamConfig::Method::ROUND_ROBIN)
            return;

        for (auto &host : hosts)
        {
            auto sock = VHostReverseProxy::connect_host(host.config);
            if (!sock)
            {
                host.up = false;
                continue;
            }
            auto req = std::make_shared<Request>("GET",
                                                 host.config.health, "1.1");
            req->set_date();
            req->headers["Host"] =
                host.config.ip + ":" + std::to_string(host.config.port);
            req->headers["Connection"] = "close";
            SendRequestEW::start(
                sock, req, std::nullopt,
                [&host](shared_res res) { host.up = res->status == OK; });
        }
    }

    UpstreamHost &Upstream::find_host()
    {
        if (config.method == UpstreamConfig::Method::FAILOVER)
            index = 0;
        auto start = index;
        do
        {
            auto &host = hosts[index];
            weight_index++;
            if (weight_index >= hosts[index].config.weight || !host.up)
            {
                weight_index = 0;
                index = (index + 1) % hosts.size();
            }
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
