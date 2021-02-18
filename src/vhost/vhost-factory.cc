#include "vhost-factory.hh"

#include "config/config.hh"
#include "vhost-static-file.hh"

namespace http
{
    shared_vhost VHostFactory::Create(VHostConfig config)
    {
        // return nullptr;
        return std::make_shared<VHostStaticFile>(config);
    }
} // namespace http
