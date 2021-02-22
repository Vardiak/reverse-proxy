#include "listener.hh"

#include <iostream>

#include "recv-request.hh"
#include "register.hh"

namespace http
{
    ListenerEW::ListenerEW(shared_socket socket, std::string ip, uint16_t port)
        : EventWatcher(socket->fd_get()->fd_, EV_READ)
        , sock_(socket)
        , ip_(ip)
        , port_(port)
    {}

    void ListenerEW::operator()()
    {
        struct sockaddr addr;
        socklen_t addrlen = sizeof(addr);

        auto client = sock_->accept(&addr, &addrlen);
        std::cout << "Client accepted!\n";

        auto conn = std::make_shared<Connection>(client, ip_, port_);

        event_register
            .register_event<RecvRequestEW, shared_socket, shared_conn>(
                std::move(client), std::move(conn));
    }
} // namespace http
