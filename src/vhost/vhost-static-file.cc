#include "vhost-static-file.hh"

namespace http
{
    VHostStaticFile::VHostStaticFile(const VHostConfig &config)
        : VHost(config)
    {}

    void VHostStaticFile::respond(Request &, std::shared_ptr<Connection>)
    {}
} // namespace http
