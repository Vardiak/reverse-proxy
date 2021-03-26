#include <criterion/criterion.h>
#include <exception>
#include <iostream>

#include "config/config.hh"
#include "error/init-error.hh"
#include "error/request-error.hh"
#include "request/request.hh"
#include "socket/default-socket.hh"
#include "vhost/connection.hh"
#include "vhost/vhost-reverse-proxy.hh"

using namespace http;

Test(forwaded_transition, single_ip)
{
    auto conn = std::make_shared<Connection>(nullptr, "0.0.0.0", 8000);

    cr_assert(Request::parse(conn) == nullptr);
    cr_assert(conn->req == nullptr);

    conn->raw += "GET /root/data/tests/static/index.html HTTP/1.1\r\nHost: "
                 "localhost\r\nX-Forwarded-For: 192.0.2.43\r\n\r\n";

    auto r1 = Request::parse(conn);
    forwarded_transition(r1);

    cr_assert(r1->headers["Forwarded"].compare("for=192.0.2.43") == 0);
}

Test(forwaded_transition, single_ipv6)
{
    auto conn = std::make_shared<Connection>(nullptr, "0.0.0.0", 8000);

    cr_assert(Request::parse(conn) == nullptr);
    cr_assert(conn->req == nullptr);

    conn->raw += "GET /root/data/tests/static/index.html HTTP/1.1\r\nHost: "
                 "localhost\r\nX-Forwarded-For: 2001:db8:cafe::17\r\n\r\n";

    auto r1 = Request::parse(conn);
    forwarded_transition(r1);
    cr_assert(r1->headers.count("Forwarded") == 1);

    cr_assert(r1->headers.count("Forwarded") == 1);
    cr_assert(r1->headers["Forwarded"].compare("for=\"[2001:db8:cafe::17]\"")
              == 0);
}

Test(forwaded_transition, double_ip)
{
    auto conn = std::make_shared<Connection>(nullptr, "0.0.0.0", 8000);

    cr_assert(Request::parse(conn) == nullptr);
    cr_assert(conn->req == nullptr);

    conn->raw +=
        "GET /root/data/tests/static/index.html HTTP/1.1\r\nHost: "
        "localhost\r\nX-Forwarded-For: 192.0.2.43, 2001:db8:cafe::17\r\n\r\n";

    auto r1 = Request::parse(conn);
    forwarded_transition(r1);
    cr_assert(r1->headers.count("Forwarded") == 1);
    cr_assert(r1->headers["Forwarded"].compare(
                  "for=192.0.2.43, for=\"[2001:db8:cafe::17]\"")
              == 0);
}

Test(forwaded_transition, ip_proto_fail)
{
    auto conn = std::make_shared<Connection>(nullptr, "0.0.0.0", 8000);

    cr_assert(Request::parse(conn) == nullptr);
    cr_assert(conn->req == nullptr);

    conn->raw += "GET /root/data/tests/static/index.html HTTP/1.1\r\nHost: "
                 "localhost\r\nX-Fowarded-For: 127.0.0.1, 133.0.0.1, "
                 "42.0.0.1\r\nX-Fowarded-Proto: http, https\r\n\r\n";

    auto r1 = Request::parse(conn);
    forwarded_transition(r1);
    cr_assert(r1->headers.count("Forwarded") == 0);
}
