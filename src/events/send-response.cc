#include "send-response.hh"

#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>

#include "error/request-error.hh"
#include "listener.hh"
#include "misc/fd.hh"
#include "misc/unistd.hh"
#include "register.hh"

namespace http
{
    SendResponseEW::SendResponseEW(shared_socket sock, shared_res response)
        : EventWatcher(sock->fd_get()->fd_, EV_WRITE)
        , sock_(sock)
        , res_(response)
        , raw_(response->to_string())
    {}

    void SendResponseEW::operator()()
    {
        // TOOD: cap size

        if (sending_file)
        {
            size_t size = res_->content_length;
            auto fd = std::make_shared<misc::FileDescriptor>(
                sys::open(res_->body.c_str(), O_RDONLY));

            off_t temp = cursor;

            try
            {
                ssize_t r =
                    sock_->sendfile(fd, temp, std::min(4096UL, size - cursor));

                if (r == 0)
                    http::event_register.unregister_ew(this);
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
                http::event_register.unregister_ew(this);
            }

            cursor = temp;

            if (cursor == size)
                http::event_register.unregister_ew(this);
        }
        else
        {
            try
            {
                size_t sent =
                    sock_->send(raw_.c_str() + cursor, raw_.length() - cursor);

                if (sent == 0)
                    http::event_register.unregister_ew(this);

                cursor += sent;
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
                http::event_register.unregister_ew(this);
            }

            if (raw_.length() == cursor)
            {
                if (res_->is_file)
                {
                    cursor = 0;
                    sending_file = true;
                }
                else
                {
                    http::event_register.unregister_ew(this);
                }
            }
        }
    }
} // namespace http
