/**
 * \file events/recv-response.hh
 * \brief EWEW declaration.
 */

#pragma once

#include "events/events.hh"
#include "request/response.hh"
#include "socket/default-socket.hh"

namespace http
{
    /**
     * \class RecvResponse
     * \brief Workflow for receiving socket.
     */
    class RecvResponseEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a RecvResponse from the shared_connection.
         */
        explicit RecvResponseEW(shared_socket sock,
                                std::function<void(shared_res)> fn,
                                std::shared_ptr<EventTimer> timeout);

        ~RecvResponseEW();

        static void start(shared_socket sock,
                          std::function<void(shared_res)> fn,
                          std::shared_ptr<EventTimer> timeout);

        void send_error_response();

        /**
         * \brief Receive me
         */
        void operator()() final;

        friend class Response;

    private:
        /**
         * \brief Client connection.
         */
        shared_socket sock_;
        std::function<void(shared_res)> callback_;
        std::shared_ptr<EventTimer> timeout_;

        std::string raw;
        size_t last = 0;
        shared_res res;
    };
} // namespace http
