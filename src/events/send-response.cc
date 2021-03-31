#include "send-response.hh"

#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>

#include "error/request-error.hh"
#include "events/recv-request.hh"
#include "listener.hh"
#include "misc/fd.hh"
#include "misc/unistd.hh"
#include "register.hh"

namespace http
{
    SendResponseEW::SendResponseEW(shared_conn conn, shared_res response)
        : EventWatcher(conn->sock_->fd_get()->fd_, EV_WRITE)
        , conn_(conn)
        , res_(response)
        , raw_(response->to_string())
    {}

    void SendResponseEW::start(shared_conn conn, shared_res res)
    {
        event_register.register_event<SendResponseEW, shared_conn, shared_res>(
            std::move(conn), std::move(res));
    }

    void SendResponseEW::send_file()
    {
        size_t size = res_->content_length;
        auto fd = std::make_shared<misc::FileDescriptor>(
            sys::open(res_->body.c_str(), O_RDONLY));

        off_t temp = cursor;

        try
        {
            ssize_t r = conn_->sock_->sendfile(fd, temp,
                                               std::min(4096UL, size - cursor));

            if (r == 0)
            {
                http::event_register.unregister_ew(this);
                return;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            http::event_register.unregister_ew(this);
            return;
        }

        cursor = temp;

        if (cursor == size)
            post_send();
    }

    void SendResponseEW::send_response()
    {
        try
        {
            size_t sent = conn_->sock_->send(raw_.c_str() + cursor,
                                             raw_.length() - cursor);

            if (sent == 0)
            {
                http::event_register.unregister_ew(this);
                return;
            }

            cursor += sent;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            http::event_register.unregister_ew(this);
            return;
        }

        if (raw_.length() == cursor)
        {
            if (res_->is_file)
            {
                cursor = 0;
                sending_file = true;
            }
            else
                post_send();
        }
    }

    void SendResponseEW::post_send()
    {
        if (res_->headers["Connection"] == "keep-alive")
        {
            auto conn = conn_;
            event_register.register_event<RecvRequestEW, shared_conn>(
                std::move(conn));
        }
        http::event_register.unregister_ew(this);
    }

    void SendResponseEW::operator()()
    {
        if (sending_file)
            send_file();
        else
            send_response();
    }
} // namespace http
