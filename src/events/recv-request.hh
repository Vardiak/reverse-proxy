/**
 * \file events/recv-request.hh
 * \brief RecvRequestEW declaration.
 */

#pragma once

#include "events/events.hh"
#include "socket/default-socket.hh"

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
         * \brief Create a RecvRequestEW from the shared_connection.
         */
        explicit RecvRequestEW(shared_conn conn);

        virtual ~RecvRequestEW();

        void send_error_response(STATUS_CODE STATUS);

        /**
         * \brief Receive me
         */
        void operator()() final;

    private:
        /**
         * \brief Client connection.
         */
        shared_conn conn_;
        std::shared_ptr<EventTimer> transaction_timeout;
        std::shared_ptr<EventTimer> keep_alive_timeout;
        std::shared_ptr<EventTimer> throughput_timeout;
        size_t throughput_bytes = 0;
    };
} // namespace http
