#include "config.hh"

#include <fstream>
#include <iostream>
#include <regex>

#include "error/init-error.hh"
#include "error/parsing-error.hh"
#include "misc/json.hh"

namespace http
{
    ServerConfig server_config;

    struct UpstreamConfig parse_upstream(const json &parsed)
    {
        struct UpstreamConfig res;
        if (!parsed.is_object())
            throw InitializationError("invalid host");

        if (!parsed.contains("method"))
            throw InitializationError("upstreams must have a method");

        std::string method = parsed["method"];
        if (method == "failover")
            res.method = UpstreamConfig::Method::FAILOVER;
        else if (method == "round-robin")
            res.method = UpstreamConfig::Method::ROUND_ROBIN;
        else if (method == "fail-robin")
            res.method = UpstreamConfig::Method::FAIL_ROBIN;
        else
            throw InitializationError("unknown upstream method");

        if (!parsed.contains("hosts") || !parsed["hosts"].is_array()
            || parsed["hosts"].size() == 0)
            throw InitializationError(
                "upstreams' hosts must be defined with at least one element");

        for (auto &parsed_host : parsed["hosts"])
        {
            UpstreamHostConfig host;

            if (!parsed_host.contains("ip"))
                throw InitializationError("host must have an ip");
            host.ip = parsed_host["ip"];

            if (!parsed_host.contains("port")
                || !parsed_host["port"].is_number_unsigned()
                || parsed_host["port"] > 65535)
                throw InitializationError("invalid host port");
            host.port = parsed_host["port"];

            if (parsed_host.contains("weight"))
                host.weight = parsed_host["weight"];

            if (res.method == UpstreamConfig::Method::FAILOVER
                || res.method == UpstreamConfig::Method::FAIL_ROBIN)
            {
                if (!parsed_host.contains("health"))
                    throw InitializationError("missing health");
                host.health = parsed_host["health"];
            }

            res.hosts.push_back(host);
        }

        return res;
    }

    struct VHostProxyPass parse_proxy_pass(const json &parsed)
    {
        VHostProxyPass proxy_pass;
        if (!parsed.is_object())
            throw InitializationError("invalid proxy_pass");

        if ((parsed.contains("ip") || parsed.contains("port"))
            && parsed.contains("upstream"))
            throw InitializationError(
                "can't have an upstream alongside ip or port");

        if (parsed.contains("upstream"))
        {
            if (!parsed["upstream"].is_string())
                throw InitializationError("upstream must be a string");

            proxy_pass.upstream = parsed["upstream"];
        }
        else
        {
            if (!(parsed.contains("ip") && parsed["ip"].is_string()
                  && parsed.contains("port") && parsed["port"].is_number()))
                throw InitializationError(
                    "ip and port must defined and valid together");

            if (!parsed["port"].is_number_unsigned() || parsed["port"] > 65535)
                throw InitializationError("invalid port proxy pass");
            proxy_pass.ip = parsed["ip"];
            proxy_pass.port = parsed["port"];
        }

        if (parsed.contains("proxy_set_header"))
            for (auto &[key, value] : parsed["proxy_set_header"].items())
                proxy_pass.proxy_set_header[key] = value;

        if (parsed.contains("proxy_remove_header"))
            for (auto &value : parsed["proxy_remove_header"])
                proxy_pass.proxy_remove_header.push_back(value);

        if (parsed.contains("set_header"))
            for (auto &[key, value] : parsed["set_header"].items())
                proxy_pass.set_header[key] = value;

        if (parsed.contains("remove_header"))
            for (auto &value : parsed["remove_header"])
                proxy_pass.remove_header.push_back(value);

        if (parsed.contains("timeout"))
        {
            if (parsed["timeout"] < 0)
                throw InitializationError("timeouts must be positive");
            proxy_pass.timeout = parsed["timeout"];
        }

        return proxy_pass;
    }

    struct VHostConfig parse_vhost(const json &parsed, bool &default_vhost)
    {
        VHostConfig vhost;
        if (!parsed.is_object())
            throw InitializationError("invalid vhost");

        if (!parsed.contains("ip") || !parsed["ip"].is_string())
            throw InitializationError("invalid ip");
        vhost.ip = parsed["ip"];

        if (!parsed.contains("port") || !parsed["port"].is_number_unsigned()
            || parsed["port"] > 65535)
            throw InitializationError("invalid port");
        vhost.port = parsed["port"];

        if (!parsed.contains("server_name")
            || !parsed["server_name"].is_string())
            throw InitializationError("invalid server name");
        vhost.server_name = parsed["server_name"];

        if (parsed.contains("proxy_pass")
            && (parsed.contains("root") || parsed.contains("default_file")
                || parsed.contains("auto_index")))
            throw InitializationError("can't have a proxy_pass alongside root, "
                                      "default_file or auto_index");

        if (parsed.contains("root"))
        {
            if (!parsed["root"].is_string())
                throw InitializationError("invalid root");
            vhost.root = parsed["root"];

            if (parsed.contains("default_file"))
            {
                if (!parsed["default_file"].is_string())
                    throw InitializationError("invalid default file");
                vhost.default_file = parsed["default_file"];
            }
            else
                vhost.default_file = "index.html";

            if (parsed.contains("auto_index"))
            {
                if (!parsed["auto_index"].is_boolean())
                    throw InitializationError("invalid auto_index");

                vhost.auto_index = parsed["auto_index"];
            }
        }
        else
        {
            if (!parsed.contains("proxy_pass"))
                throw InitializationError("no vhost type");
            vhost.proxy_pass = parse_proxy_pass(parsed["proxy_pass"]);
        }

        if ((parsed.contains("auth_basic") && parsed["auth_basic"].is_string())
            != (parsed.contains("auth_basic_users")
                && parsed["auth_basic_users"].is_array()))
            throw InitializationError(
                "auth_basic and auth_basic_users must be defined together");

        if (parsed.contains("auth_basic"))
        {
            vhost.auth_basic = parsed["auth_basic"];
            if (vhost.auth_basic.empty() || parsed["auth_basic_users"].empty())
                throw InitializationError("Empty auth");
            for (std::string user : parsed["auth_basic_users"])
            {
                const std::regex auth(".+:.+");
                if (!std::regex_match(user, auth))
                    throw InitializationError(
                        "Basic users' syntax must be username:password");
                vhost.auth_basic_users.insert(user);
            }
        }

        if (parsed.contains("default_vhost"))
        {
            if (default_vhost || !parsed["default_vhost"].is_boolean())
                throw InitializationError(
                    "Default vhost must be unique and valid");
            vhost.default_vhost = true;
            default_vhost = true;
        }

        if ((parsed.contains("ssl_cert") && parsed["ssl_cert"].is_string())
            != (parsed.contains("ssl_key") && parsed["ssl_key"].is_string()))
            throw InitializationError(
                "ssl_cert and ssl_key must be defined together");
        if (parsed.contains("ssl_cert"))
        {
            vhost.ssl_cert = parsed["ssl_cert"];
            vhost.ssl_key = parsed["ssl_key"];
            if (vhost.ssl_cert.empty() || vhost.ssl_cert.empty())
                throw InitializationError("empty string ssl");
        }

        return vhost;
    }

    Timeout parse_timeout(const json &parsed)
    {
        Timeout timeout;

        if (!parsed.is_object())
            throw InitializationError("Invalid timeout");

        if (parsed.contains("keep_alive"))
        {
            if (parsed["keep_alive"] < 0)
                throw InitializationError("timeouts must be positive");
            timeout.keep_alive = parsed["keep_alive"];
        }

        if (parsed.contains("transaction"))
        {
            if (parsed["transaction"] < 0)
                throw InitializationError("timeouts must be positive");
            timeout.transaction = parsed["transaction"];
        }

        if (parsed.contains("throughput_val")
            != parsed.contains("throughput_time"))
            throw InitializationError(
                "throughput_val and throughput_time must be defined together");
        else if (parsed.contains("throughput_val"))
        {
            if (parsed["throughput_time"] < 0)
                throw InitializationError("timeouts must be positive");
            if (parsed["throughput_val"] < 0)
                throw InitializationError("throughput val must be positive");
            timeout.throughput_time = parsed["throughput_time"];
            timeout.throughput_val = parsed["throughput_val"];
        }

        return timeout;
    }

    void parse_configuration(const std::string &path)
    {
        std::ifstream file(path);
        json parsed;
        // Throws parsing error if invalid
        file >> parsed;

        bool default_vhost = false;

        if (!(parsed.is_object() && parsed["vhosts"].is_array()
              && parsed["vhosts"].size() > 0))
            throw InitializationError("couldn't find vhosts in config");

        for (auto vhost_parsed : parsed["vhosts"])
            server_config.vhosts.push_back(
                parse_vhost(vhost_parsed, default_vhost));

        for (auto &[key, value] : parsed["upstreams"].items())
            server_config.upstreams[key] = parse_upstream(value);

        if (parsed.contains("timeout"))
            server_config.timeout = parse_timeout(parsed["timeout"]);
    }
} // namespace http
