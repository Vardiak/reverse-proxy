/**
 * \file events/send-response.hh
 * \brief SendResponseEW declaration.
 */

#pragma once

#include "events/events.hh"
#include "request/response.hh"
#include "socket/default-socket.hh"
#include "socket/socket.hh"

namespace http
{
    /**
     * \class SendResponseEW
     * \brief Workflow for receiving socket.
     */
    class SendResponseEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a SendResponseEW from the shared_socket.
         */
        explicit SendResponseEW(shared_socket sock, shared_res response);

        /**
         * \brief Receive me
         */
        void operator()() final;

    private:
        /**
         * \brief Client socket.
         */
        shared_socket sock_;
        shared_res res_;

        std::string raw_;

        size_t cursor = 0;
        bool sending_file = false;
    };
} // namespace http
