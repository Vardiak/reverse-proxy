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
    SendRequestEW::SendRequestEW(shared_socket sock, shared_req req,
                                 std::function<void(shared_res)> fn,
                                 std::optional<float> time)
        : EventWatcher(sock->fd_get()->fd_, EV_WRITE)
        , sock_(sock)
        , req_(req)
        , raw_(req->to_string())
        , callback_(fn)
    {
        if (time)
        {
            this->timeout_ =
                EventTimer::start(this, time.value(), 0, [this, fn]() {
                    fn(std::make_shared<Response>(GATEWAY_TIMEOUT));
                    return true;
                });
        }
    }

    SendRequestEW::~SendRequestEW()
    {
        if (timeout_)
            event_register.unregister_et(timeout_.get());
    }

    void SendRequestEW::start(shared_socket sock, shared_req req,
                              std::optional<float> time,
                              std::function<void(shared_res)> fn)
    {
        http::event_register.register_event<
            SendRequestEW, shared_socket, shared_req,
            std::function<void(shared_res)>, std::optional<float>>(
            std::move(sock), std::move(req), std::move(fn), std::move(time));
    }

    void SendRequestEW::post_send()
    {
        RecvResponseEW::start(sock_, callback_, timeout_);
        timeout_ = nullptr;
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
