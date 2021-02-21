#include "dispatcher.hh"

#include <string>

#include "error/request-error.hh"
#include "vhost.hh"

namespace http
{
    void Dispatcher::add_vhost(shared_vhost vhost)
    {
        std::string key = vhost->conf_get().server_name + ':'
            + vhost->conf_get().ip + ':'
            + std::to_string(vhost->conf_get().port);
        vhosts_[key] = vhost;
    }

    void Dispatcher::dispatch(Request &r, std::shared_ptr<Connection> conn)
    {
        if (r.headers.count("Host") == 0)
            throw RequestError(BAD_REQUEST);

        std::string key = r.headers["Host"] + ':' + conn->ip_ + ':'
            + std::to_string(conn->port_);

        if (vhosts_.count(key) == 0)
            throw RequestError(NOT_FOUND);

        vhosts_[key].respond(r, conn);
    }
} // namespace http
