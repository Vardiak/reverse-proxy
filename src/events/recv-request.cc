#include "recv-request.hh"

#include <optional>
#include <vector>

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

        connection_->store.append(buffer, read);

        while (auto request = Request::parse(connection_))
        {
            // send request to dispatcher
        }
    }

} // namespace http
