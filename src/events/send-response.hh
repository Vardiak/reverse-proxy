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
        explicit SendResponseEW(shared_conn conn, shared_res response);

        static void start(shared_conn conn, shared_res res);

        /**
         * \brief Receive me
         */
        void operator()() final;

        void send_file();
        void send_response();
        void post_send();

    private:
        /**
         * \brief Client socket.
         */
        shared_conn conn_;
        shared_res res_;

        std::string raw_;

        size_t cursor = 0;
        bool sending_file = false;
    };
} // namespace http
