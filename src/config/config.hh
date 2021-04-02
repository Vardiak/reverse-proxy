/**
 * \file config/config.hh
 * \brief Declaration of ServerConfig and VHostConfig.
 */
#pragma once

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "misc/json_fwd.hh"

namespace http
{
    struct VHostProxyPass
    {
        std::string ip;
        uint16_t port;
        std::string upstream;
        std::map<std::string, std::string> proxy_set_header;
        std::vector<std::string> proxy_remove_header;
        std::map<std::string, std::string> set_header;
        std::vector<std::string> remove_header;
        std::optional<float> timeout;
    };

    /**
     * \struct VHostConfig
     * \brief Value object storing a virtual host configuration.
     *
     * Since each virtual host of the server has its own configuration, a
     * dedicated structure is required to store the information related to
     * each one of them.
     */
    struct VHostConfig
    {
        VHostConfig() = default;
        VHostConfig(const VHostConfig &) = default;
        VHostConfig &operator=(const VHostConfig &) = default;
        VHostConfig(VHostConfig &&) = default;
        VHostConfig &operator=(VHostConfig &&) = default;

        ~VHostConfig() = default;

        std::string ip;
        uint16_t port;
        std::string server_name;
        std::string root;
        std::string default_file;
        std::string auth_basic;
        std::set<std::string> auth_basic_users;
        std::string ssl_cert;
        std::string ssl_key;
        bool default_vhost = false;
        std::optional<VHostProxyPass> proxy_pass;
        bool auto_index = false;
    };

    struct UpstreamHostConfig
    {
        std::string ip;
        uint16_t port;
        uint weight = 1;
        std::string health;
    };

    struct UpstreamConfig
    {
        enum class Method
        {
            FAILOVER,
            ROUND_ROBIN,
            FAIL_ROBIN
        };
        Method method;
        std::vector<UpstreamHostConfig> hosts;
    };

    struct Timeout
    {
        Timeout() = default;
        Timeout(const Timeout &) = default;
        Timeout &operator=(const Timeout &) = default;
        Timeout(Timeout &&) = default;
        Timeout &operator=(Timeout &&) = default;

        ~Timeout() = default;

        std::optional<float> keep_alive;
        std::optional<float> transaction;
        std::optional<unsigned int> throughput_val;
        std::optional<float> throughput_time;
    };

    /**
     * \struct ServerConfig
     * \brief Value object storing the server configuration.
     *
     * To avoid opening the configuration file each time we need to access the
     * server configuration, a dedicated structure is required to store it.
     */
    struct ServerConfig
    {
        ServerConfig() = default;
        ServerConfig(const ServerConfig &) = default;
        ServerConfig &operator=(const ServerConfig &) = default;
        ServerConfig(ServerConfig &&) = default;
        ServerConfig &operator=(ServerConfig &&) = default;

        ~ServerConfig() = default;

        std::vector<VHostConfig> vhosts;
        std::map<std::string, UpstreamConfig> upstreams;
        std::optional<Timeout> timeout;
    };

    /**
     * \brief Parse the server configuration file.
     *
     * \param path string containing the path to the server configuration
     * file.
     * \return The server configuration.
     */
    struct VHostProxyPass parse_proxy_pass(const json &parsed);
    struct VHostConfig parse_vhost(const json &parsed, bool &default_vhost);
    struct UpstreamConfig parse_upstream(const json &parsed);
    struct Timeout parse_timeout(const json &parsed);
    void parse_configuration(const std::string &path);

    extern ServerConfig server_config;
} // namespace http
