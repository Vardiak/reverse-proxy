#include "config.hh"

#include <fstream>

#include "error/init-error.hh"
#include "error/parsing-error.hh"
#include "misc/json.hh"

namespace http
{
    struct ServerConfig parse_configuration(const std::string &path)
    {
        ServerConfig config;

        std::ifstream file(path);
        json parsed;
        // Throw parsing error if invalid
        file >> parsed;

        if (!(parsed.is_object() && parsed["vhosts"].is_array()
              && parsed["vhosts"].size() > 0))
            throw http::InitializationError("couldn't find vhosts in config");

        for (json vhost_parsed : parsed["vhosts"])
        {
            if (!vhost_parsed.is_object())
                throw http::InitializationError("invalid vhost");
            else if (!vhost_parsed["ip"].is_string())
                throw http::InitializationError("invalid ip");
            else if (!vhost_parsed["port"].is_number_integer())
                throw http::InitializationError("invalid port");
            else if (!vhost_parsed["server_name"].is_string())
                throw http::InitializationError("invalid server name");
            else if (!vhost_parsed["root"].is_string())
                throw http::InitializationError("invalid root");
            else if (vhost_parsed.contains("default_file")
                     && !vhost_parsed["default_file"].is_string())
                throw http::InitializationError("invalid default file");

            VHostConfig vhost;
            vhost.server_name = vhost_parsed["server_name"];
            vhost.ip = vhost_parsed["ip"];
            vhost.port = vhost_parsed["port"];
            vhost.root = vhost_parsed["root"];
            if (vhost_parsed.contains("default_file"))
                vhost.default_file = vhost_parsed["default_file"];
            else
                vhost.default_file = "index.html";

            config.vhosts.push_back(vhost);
        }

        return config;
    }
} // namespace http
