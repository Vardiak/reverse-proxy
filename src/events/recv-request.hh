/**
 * \file events/listener.hh
 * \brief ListenerEW declaration.
 */

#pragma once

#include "events/events.hh"
#include "socket/default-socket.hh"
#include "socket/socket.hh"

namespace http
{
    /**
     * \class RecvRequestEW
     * \brief Workflow for receiving socket.
     */
    class RecvRequestEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a RecvRequestEW from the shared_socket.
         */
        explicit RecvRequestEW(shared_socket socket);

        /**
         * \brief Receive me
         */
        void operator()() final;

    private:
        /**
         * \brief Client socket.
         */
        shared_socket sock_;
        std::shared_ptr<Connection> connection_;
    };
} // namespace http
