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

        try
        {
            auto client = sock_->accept(&addr, &addrlen);
            // Set socket non blocking
            int flags = fcntl(client->fd_get()->fd_, F_GETFL);
            fcntl(client->fd_get()->fd_, F_SETFL, flags | O_NONBLOCK);

            auto conn = std::make_shared<Connection>(client, ip_, port_);

            event_register
                .register_event<RecvRequestEW, shared_socket, shared_conn>(
                    std::move(client), std::move(conn));
        }
        catch (const std::system_error &e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
} // namespace http
