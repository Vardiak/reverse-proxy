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

        bool default_vhost = false;

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
            else if (vhost_parsed["auth_basic"].is_string()
                     != vhost_parsed["auth_basic_users"].is_array())
                throw http::InitializationError(
                    "auth_basic and auth_basic_users must be defined together");
            else if (vhost_parsed.contains("default_vhost")
                     && (default_vhost
                         || !vhost_parsed["default_vhost"].is_string()))
                throw http::InitializationError(
                    "Default vhost must be unique and valid");
            else if (vhost_parsed["ssl_cert"].is_string()
                     != vhost_parsed["ssl_key"].is_string())
                throw http::InitializationError(
                    "ssl_cert and ssl_key must be defined together");

            VHostConfig vhost;
            vhost.server_name = vhost_parsed["server_name"];
            vhost.ip = vhost_parsed["ip"];
            vhost.port = vhost_parsed["port"];
            vhost.root = vhost_parsed["root"];
            if (vhost_parsed.contains("default_file"))
                vhost.default_file = vhost_parsed["default_file"];
            else
                vhost.default_file = "index.html";

            // if (vhost_parsed.contains("auth_basic"))
            // {
            //     vhost.auth_basic = vhost_parsed["auth_basic"];
            //     vhost.auth_basic_users = vhost_parsed["auth_basic_users"];
            // }
            // if (vhost_parsed.contains("ssl_cert"))
            // {
            //     vhost.ssl_cert = vhost_parsed["ssl_cert"];
            //     vhost.ssl_key = vhost_parsed["ssl_key"];
            // }
            // if (vhost_parsed.contains("default_vhost"))
            // {
            //     vhost.default_vhost = true;
            //     default_vhost = true;
            // }

            config.vhosts.push_back(vhost);
        }

        return config;
    }
} // namespace http
