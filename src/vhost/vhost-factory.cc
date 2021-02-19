#include "vhost-factory.hh"

#include "config/config.hh"
#include "vhost-static-file.hh"

namespace http
{
    shared_vhost VHostFactory::Create(VHostConfig config)
    {
        auto vhost = new VHostStaticFile(config);
        return std::shared_ptr<VHostStaticFile>(vhost);
    }
} // namespace http
