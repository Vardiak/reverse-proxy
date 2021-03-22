/**
 * \file events/send-request.hh
 * \brief SendRequestEW declaration.
 */

#pragma once

#include "events/events.hh"
#include "request/request.hh"
#include "socket/default-socket.hh"
#include "socket/socket.hh"

namespace http
{
    /**
     * \class SendRequestEW
     * \brief Workflow for receiving socket.
     */
    class SendRequestEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a SendRequestEW from the shared_socket.
         */
        explicit SendRequestEW(shared_socket sock, shared_req req,
                               std::function<void(shared_res)> fn)
            : EventWatcher(sock->fd_get()->fd_, EV_WRITE)
            , sock_(sock)
            , req_(req)
            , raw_(req->to_string())
            , callback_(fn)
        {}

        static void start(shared_socket sock, shared_req req,
                          std::function<void(shared_res)> fn);

        /**
         * \brief Receive me
         */
        void operator()() final;

        void post_send();

    private:
        /**
         * \brief Backend socket.
         */
        shared_socket sock_;
        shared_req req_;

        std::string raw_;

        size_t cursor = 0;

        std::function<void(shared_res)> callback_;
    };
} // namespace http
