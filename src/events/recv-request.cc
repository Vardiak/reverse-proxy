#include "recv-request.hh"

#include <optional>
#include <vector>

#include "error/request-error.hh"
#include "listener.hh"
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

        connection_->raw.append(buffer, read);

        try
        {
            while (auto request = Request::parse(connection_))
            {
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
