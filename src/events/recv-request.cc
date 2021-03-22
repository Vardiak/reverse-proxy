#include "recv-request.hh"

#include <iostream>
#include <optional>
#include <vector>

#include "error/request-error.hh"
#include "listener.hh"
#include "misc/unistd.hh"
#include "register.hh"
#include "request/request.hh"
#include "send-response.hh"
#include "vhost/dispatcher.hh"

namespace http
{
    RecvRequestEW::RecvRequestEW(shared_conn conn)
        : EventWatcher(conn->sock_->fd_get()->fd_, EV_READ)
        , conn_(conn)
    {}

    void RecvRequestEW::send_error_response(STATUS_CODE status)
    {
        auto res = std::make_shared<Response>(status);

        auto conn = conn_;
        event_register.register_event<SendResponseEW, shared_conn, shared_res>(
            std::move(conn), std::move(res));

        event_register.unregister_ew(this);
    }

    void RecvRequestEW::operator()()
    {
        char buffer[4096];

        std::cout << "recv" << std::endl;
        ssize_t read = conn_->sock_->recv(buffer, sizeof(buffer));
        if (read == 0)
        {
            event_register.unregister_ew(this);
            return;
        }

        conn_->raw.append(buffer, read);

        try
        {
            if (auto request = Request::parse(conn_))
            {
                std::cout << "parsed" << std::endl;

                dispatcher.dispatch(request, conn_);

                event_register.unregister_ew(this);
            }
        }
        catch (const RequestError &e)
        {
            // Send response & close connection

            send_error_response(e.status);
        }
        catch (const std::exception &e)
        {
            std::cout << "Internal server error : " << e.what() << std::endl;

            send_error_response(INTERNAL_SERVER_ERROR);
        }
    }

} // namespace http
