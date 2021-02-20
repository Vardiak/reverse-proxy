#include "listener.hh"

#include <iostream>

#include "recv-request.hh"
#include "register.hh"

namespace http
{
    ListenerEW::ListenerEW(shared_socket socket)
        : EventWatcher(socket->fd_get()->fd_, EV_READ)
        , sock_(socket)
    {}

    void ListenerEW::operator()()
    {
        struct sockaddr addr;
        socklen_t addrlen = sizeof(addr);

        auto client = sock_->accept(&addr, &addrlen);
        std::cout << "Client accepted!\n";

        event_register.register_event<RecvRequestEW, shared_socket>(
            std::move(client));
    }
} // namespace http
