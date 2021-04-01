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
                               std::function<void(shared_res)> fn,
                               std::optional<float> time);

        ~SendRequestEW();

        static void start(shared_socket sock, shared_req req,
                          std::optional<float> time,
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
        std::shared_ptr<EventTimer> timeout_;
    };
} // namespace http
