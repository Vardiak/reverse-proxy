#include "recv-request.hh"

#include <iostream>
#include <optional>
#include <vector>

#include "error/request-error.hh"
#include "listener.hh"
#include "misc/unistd.hh"
#include "register.hh"
#include "request/request.hh"

namespace http
{
    RecvRequestEW::RecvRequestEW(shared_socket socket)
        : EventWatcher(socket->fd_get()->fd_, EV_READ)
        , sock_(socket)
        , connection_(std::make_shared<Connection>())
    {}

    void RecvRequestEW::operator()()
    {
        char buffer[512];

        ssize_t read = sock_->recv(buffer, sizeof(buffer));
        if (read == 0)
        {
            event_register.unregister_ew(this);
            return;
        }

        std::cout << "Received buffer of size " << read << std::endl;

        connection_->raw.append(buffer, read);

        std::cout << "Received input\n";

        try
        {
            while (auto request = Request::parse(connection_))
            {
                std::cout << "Complete request\n";

                // send request to dispatcher
                // close connection if not keep alive
            }
        }
        catch (const RequestError &e)
        {
            // Send response & close connection
        }
    }

} // namespace http
