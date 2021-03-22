#include "vhost-factory.hh"

#include "config/config.hh"
#include "vhost-reverse-proxy.hh"
#include "vhost-static-file.hh"

namespace http
{
    shared_vhost VHostFactory::Create(VHostConfig config)
    {
        VHost *vhost = nullptr;
        if (config.proxy_pass)
            vhost = new VHostReverseProxy(config);
        else
            vhost = new VHostStaticFile(config);
        return std::shared_ptr<VHost>(vhost);
    }
} // namespace http
