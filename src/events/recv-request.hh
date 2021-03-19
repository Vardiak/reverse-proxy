/**
 * \file events/recv-request.hh
 * \brief RecvRequestEW declaration.
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
         * \brief Create a RecvRequestEW from the shared_connection.
         */
        explicit RecvRequestEW(shared_conn conn);

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
    };
} // namespace http
