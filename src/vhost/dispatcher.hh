/**
 * \file vhost/dispatcher.hh
 * \brief Dispatcher declaration.
 */

#pragma once
#include <map>
#include <optional>

#include "vhost-static-file.hh"

namespace http
{
    /**
     * \class Dispatcher
     * \brief Instance in charge of dispatching requests between vhosts.
     */
    class Dispatcher
    {
    public:
        Dispatcher() = default;
        Dispatcher(const Dispatcher &) = delete;
        Dispatcher &operator=(const Dispatcher &) = delete;
        Dispatcher(Dispatcher &&) = delete;
        Dispatcher &operator=(Dispatcher &&) = delete;

        void add_vhost(shared_vhost);
        void dispatch(Request &, std::shared_ptr<Connection>);
        std::optional<shared_vhost> find_vhost(std::string server_name,
                                               std::string ip, uint16_t port);

    private:
        /* FIXME: Add members to store the information relative to the
        ** Dispatcher.
        */
        std::map<std::string, shared_vhost> vhosts_;
        std::optional<shared_vhost> default_;
    };

    /**
     * \brief Service object.
     */
    extern Dispatcher dispatcher;
} // namespace http
