#include "send-request.hh"

#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>

#include "error/request-error.hh"
#include "events/recv-response.hh"
#include "listener.hh"
#include "misc/fd.hh"
#include "misc/unistd.hh"
#include "register.hh"
#include "request/error.hh"

namespace http
{
    void SendRequestEW::start(shared_socket sock, shared_req req,
                              std::function<void(shared_res)> fn)
    {
        http::event_register
            .register_event<SendRequestEW, shared_socket, shared_req,
                            std::function<void(shared_res)>>(
                std::move(sock), std::move(req), std::move(fn));
    }

    void SendRequestEW::post_send()
    {
        RecvResponseEW::start(sock_, callback_);
        http::event_register.unregister_ew(this);
    }

    void SendRequestEW::operator()()
    {
        try
        {
            size_t sent =
                sock_->send(raw_.c_str() + cursor, raw_.length() - cursor);

            if (sent == 0)
            {
                callback_(std::make_shared<Response>(BAD_GATEWAY));
                http::event_register.unregister_ew(this);
                return;
            }

            cursor += sent;
        }
        catch (const std::exception &e)
        {
            callback_(std::make_shared<Response>(BAD_GATEWAY));
            http::event_register.unregister_ew(this);
            return;
        }

        if (raw_.length() == cursor)
            post_send();
    }
} // namespace http
