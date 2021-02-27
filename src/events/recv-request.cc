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
    RecvRequestEW::RecvRequestEW(shared_socket sock, shared_conn conn)
        : EventWatcher(sock->fd_get()->fd_, EV_READ)
        , sock_(sock)
        , conn_(conn)
    {}

    void RecvRequestEW::operator()()
    {
        char buffer[4096];

        ssize_t read = sock_->recv(buffer, sizeof(buffer));
        if (read == 0)
        {
			std::cout << "Unregistering client\n";
            event_register.unregister_ew(this);
            return;
        }

        std::cout << "Received buffer of size " << read << std::endl;

        conn_->raw.append(buffer, read);

        std::cout << "Received input\n";

        try
        {
            while (auto request = Request::parse(conn_))
            {
                std::cout << "Complete request\n";

                Request r = *(request.value().get());

                dispatcher.dispatch(r, conn_);

                // TODO: handle multiple requests
                event_register.unregister_ew(this);
            }
        }
        catch (const RequestError &e)
        {
            std::cout << "Got error : " << e.what() << std::endl;
            // Send response & close connection

            auto res = std::make_shared<Response>(e.status);

            auto sock = sock_;
            event_register
                .register_event<SendResponseEW, shared_socket, shared_res>(
                    std::move(sock), std::move(res));

            event_register.unregister_ew(this);
        }
        catch (const std::exception &e)
        {
            std::cout << "Internal server error : " << e.what() << std::endl;

            auto res = std::make_shared<Response>(INTERNAL_SERVER_ERROR);

            auto sock = sock_;
            event_register
                .register_event<SendResponseEW, shared_socket, shared_res>(
                    std::move(sock), std::move(res));

            event_register.unregister_ew(this);
        }
    }

} // namespace http
