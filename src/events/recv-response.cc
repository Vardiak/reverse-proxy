#include "recv-response.hh"

#include <iostream>
#include <vector>

#include "listener.hh"
#include "misc/unistd.hh"
#include "register.hh"
#include "vhost/dispatcher.hh"

namespace http
{
    RecvResponseEW::RecvResponseEW(shared_socket sock,
                                   std::function<void(shared_res)> fn,
                                   std::shared_ptr<EventTimer> timeout)
        : EventWatcher(sock->fd_get()->fd_, EV_READ)
        , sock_(sock)
        , callback_(fn)
        , timeout_(timeout)
    {}

    RecvResponseEW::~RecvResponseEW()
    {
        if (timeout_)
            event_register.unregister_et(timeout_.get());
    }

    void RecvResponseEW::start(shared_socket sock,
                               std::function<void(shared_res)> fn,
                               std::shared_ptr<EventTimer> timeout)
    {
        http::event_register.register_event<RecvResponseEW, shared_socket,
                                            std::function<void(shared_res)>,
                                            std::shared_ptr<EventTimer>>(
            std::move(sock), std::move(fn), std::move(timeout));
    }

    void RecvResponseEW::send_error_response()
    {
        callback_(std::make_shared<Response>(BAD_GATEWAY));
        event_register.unregister_ew(this);
    }

    void RecvResponseEW::operator()()
    {
        char buffer[4096];

        ssize_t read = sock_->recv(buffer, sizeof(buffer));
        if (read == 0)
        {
            send_error_response();
            return;
        }

        raw.append(buffer, read);

        try
        {
            if (Response::parse(*this))
            {
                callback_(res);
                event_register.unregister_ew(this);
            }
        }
        catch (const std::exception &e)
        {
            std::cout << "Internal server error : " << e.what() << std::endl;
            send_error_response();
        }
    }

} // namespace http
