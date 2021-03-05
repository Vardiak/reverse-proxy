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
        std::string key = vhost->conf_get().server_name + '@'
            + vhost->conf_get().ip + '@'
            + std::to_string(vhost->conf_get().port);

        if (vhosts_.count(key) > 0)
            throw InitializationError("Multiple vhosts with same signature.");

        vhosts_[key] = vhost;

        std::string key2 = vhost->conf_get().ip + '@' + vhost->conf_get().ip
            + '@' + std::to_string(vhost->conf_get().port);

        if (vhosts_.count(key2) > 0 && vhosts_[key2]->conf_get().default_vhost)
            return;

        vhosts_[key2] = vhost;
    }

    void Dispatcher::dispatch(Request &r, std::shared_ptr<Connection> conn)
    {
        if (r.headers.count("Host") == 0)
            throw RequestError(BAD_REQUEST);

        // Remove port from host
        std::string domain =
            r.headers["Host"].substr(0, r.headers["Host"].find(':'));

        std::string key =
            domain + '@' + conn->ip_ + '@' + std::to_string(conn->port_);

        if (vhosts_.count(key) == 0)
        {
            const std::regex ip_regex("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");
            if (!std::regex_match(domain, ip_regex))
                throw RequestError(NOT_FOUND);

            key = "0.0.0.0@0.0.0.0@" + std::to_string(conn->port_);
            if (vhosts_.count(key) == 0)
                throw RequestError(NOT_FOUND);
        }

        vhosts_[key]->respond(r, conn);
    }
} // namespace http
