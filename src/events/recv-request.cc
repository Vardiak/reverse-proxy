#include "recv-request.hh"

#include <iostream>
#include <optional>
#include <vector>

#include "error/request-error.hh"
#include "listener.hh"
#include "misc/unistd.hh"
#include "register.hh"
#include "request/request.hh"
#include "send-response.hh"
#include "vhost/dispatcher.hh"

namespace http
{
    RecvRequestEW::RecvRequestEW(shared_conn conn)
        : EventWatcher(conn->sock_->fd_get()->fd_, EV_READ)
        , conn_(conn)
    {
        if (server_config.timeout && server_config.timeout->keep_alive)
        {
            keep_alive_timeout = EventTimer::start(
                this, server_config.timeout->keep_alive.value(), 0, [this]() {
                    auto res = std::make_shared<Response>(REQUEST_TIMEOUT);
                    res->headers["X-Timeout-Reason"] = "Keep-Alive";

                    SendResponseEW::start(conn_, res);
                    return true;
                });
        }
    }

    RecvRequestEW::~RecvRequestEW()
    {
        if (transaction_timeout)
            event_register.unregister_et(transaction_timeout.get());
        if (keep_alive_timeout)
            event_register.unregister_et(keep_alive_timeout.get());
        if (throughput_timeout)
            event_register.unregister_et(throughput_timeout.get());
    }

    void RecvRequestEW::send_error_response(STATUS_CODE status)
    {
        auto res = std::make_shared<Response>(status);

        SendResponseEW::start(conn_, res);

        event_register.unregister_ew(this);
    }

    void RecvRequestEW::operator()()
    {
        char buffer[4096];

        if (keep_alive_timeout)
        {
            event_register.unregister_et(keep_alive_timeout.get());
            keep_alive_timeout = nullptr;
        }

        if (server_config.timeout && server_config.timeout->transaction
            && !transaction_timeout)
        {
            transaction_timeout = EventTimer::start(
                this, server_config.timeout->transaction.value(), 0, [this]() {
                    auto res = std::make_shared<Response>(REQUEST_TIMEOUT);
                    res->headers["X-Timeout-Reason"] = "Transaction";

                    SendResponseEW::start(conn_, res);
                    return true;
                });
        }

        if (server_config.timeout && server_config.timeout->throughput_val
            && !throughput_timeout)
        {
            float interval = server_config.timeout->throughput_time.value();
            throughput_timeout =
                EventTimer::start(this, interval, interval, [this]() {
                    if (this->throughput_bytes
                        >= server_config.timeout->throughput_val.value())
                    {
                        this->throughput_bytes = 0;
                        return false;
                    }
                    auto res = std::make_shared<Response>(REQUEST_TIMEOUT);
                    res->headers["X-Timeout-Reason"] = "Throughput";

                    SendResponseEW::start(conn_, res);
                    return true;
                });
        }

        ssize_t read = conn_->sock_->recv(buffer, sizeof(buffer));
        throughput_bytes += read;

        if (read == 0)
        {
            event_register.unregister_ew(this);
            return;
        }

        conn_->raw.append(buffer, read);

        try
        {
            if (auto request = Request::parse(conn_))
            {
                dispatcher.dispatch(request, conn_);

                event_register.unregister_ew(this);
            }
        }
        catch (const RequestError &e)
        {
            // Send response & close connection

            send_error_response(e.status);
        }
        catch (const std::exception &e)
        {
            std::cout << "Internal server error : " << e.what() << std::endl;

            send_error_response(INTERNAL_SERVER_ERROR);
        }
    }

} // namespace http
