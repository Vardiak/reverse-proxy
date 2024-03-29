#include "dispatcher.hh"

#include <iostream>
#include <regex>
#include <string>

#include "error/init-error.hh"
#include "error/request-error.hh"
#include "vhost.hh"

namespace http
{
    Dispatcher dispatcher;

    void Dispatcher::add_vhost(shared_vhost vhost)
    {
        if (vhost->conf_get().default_vhost)
            default_ = vhost;

        if (vhost->conf_get().ip == "::"
            || vhost->conf_get().ip == "0:0:0:0:0:0:0:0"
            || vhost->conf_get().ip == "::0")
            vhost->conf_get().ip = "0::0";

        std::string wildcard =
            vhost->conf_get().ip.find(':') != std::string::npos ? "0::0"
                                                                : "0.0.0.0";

        std::string key = vhost->conf_get().server_name + '@'
            + vhost->conf_get().ip + '@'
            + std::to_string(vhost->conf_get().port);
        std::string wildcard_key = vhost->conf_get().server_name + '@'
            + wildcard + '@' + std::to_string(vhost->conf_get().port);

        if (vhosts_.count(key) > 0 || vhosts_.count(wildcard_key) > 0)
            throw InitializationError("Multiple vhosts with same signature.");

        vhosts_[key] = vhost;

        std::string key2 = vhost->conf_get().ip + '@' + vhost->conf_get().ip
            + '@' + std::to_string(vhost->conf_get().port);

        if (vhosts_.count(key2) > 0 && vhosts_[key2]->conf_get().default_vhost)
            return;

        vhosts_[key2] = vhost;
    }

    void Dispatcher::dispatch(shared_req &r, std::shared_ptr<Connection> conn)
    {
        if (r->headers.count("Host") == 0)
            throw RequestError(BAD_REQUEST);

        // Remove port from host
        std::string domain =
            r->headers["Host"].substr(0, r->headers["Host"].find(':'));

        auto vhost = find_vhost(domain, conn->ip_, conn->port_);
        if (!vhost || (conn->vhost && conn->vhost != vhost.value()))
            throw RequestError(BAD_REQUEST);
        else if (!conn->vhost)
            conn->vhost = vhost.value();

        vhost.value()->respond(r, conn);
    }

    std::optional<shared_vhost> Dispatcher::find_vhost(std::string server_name,
                                                       std::string ip,
                                                       uint16_t port)
    {
        std::string wildcard =
            ip.find(':') != std::string::npos ? "0::0" : "0.0.0.0";
        std::string key = server_name + '@' + ip + '@' + std::to_string(port);

        if (vhosts_.count(key) == 0)
        {
            key = server_name + "@" + wildcard + "@" + std::to_string(port);

            if (vhosts_.count(key) == 0)
            {
                const std::regex ip_regex("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");
                if (!std::regex_match(server_name, ip_regex))
                    return default_;
                key = wildcard + "@" + wildcard + "@" + std::to_string(port);

                if (vhosts_.count(key) == 0)
                    return default_;
            }
        }
        return vhosts_[key];
    }
} // namespace http
