#include "request/request.hh"

#include <criterion/criterion.h>
#include <exception>
#include <iostream>

#include "config/config.hh"
#include "error/init-error.hh"
#include "error/request-error.hh"
#include "socket/default-socket.hh"
#include "vhost/connection.hh"

using namespace http;

Test(request_line, success)
{
    std::string s = "GET /hello.htm HTTP/1.1";
    auto req = Request::parse_request_line(s);

    cr_assert(req->method == METHOD::GET);
    cr_assert(req->target.compare("/hello.htm") == 0);
    cr_assert(req->http_version.compare("1.1") == 0);
}

Test(request_line, success_post)
{
    std::string s = "POST /hello.htm HTTP/1.1";
    auto req = Request::parse_request_line(s);

    cr_assert(req->method == METHOD::POST);
    cr_assert(req->target.compare("/hello.htm") == 0);
    cr_assert(req->http_version.compare("1.1") == 0);
}

Test(request_line, method_fail)
{
    std::string s = "PTnDETESTDEMERDE /hello.htm HTTP/1.1";
    try
    {
        Request::parse_request_line(s);
        cr_assert(false);
    }
    catch (const RequestError &e)
    {
        cr_assert_eq(e.status, NOT_IMPLEMENTED);
    }
}

Test(request_line, http_version_fail)
{
    std::string s = "GET /hello.htm hachetétépé/unpointun";
    try
    {
        Request::parse_request_line(s);
        cr_assert(false);
    }
    catch (const RequestError &e)
    {
        cr_assert_eq(e.status, BAD_REQUEST);
    }
}

Test(request_line, http_version_unsupported)
{
    std::string s = "GET /hello.htm HTTP/2.0";
    try
    {
        Request::parse_request_line(s);
        cr_assert(false);
    }
    catch (const RequestError &e)
    {
        cr_assert_eq(e.status, HTTP_VERSION_NOT_SUPPORTED);
    }
}

Test(request_line, http_minor_version_unsupported)
{
    std::string s = "GET /hello.htm HTTP/1.2";
    try
    {
        Request::parse_request_line(s);
        cr_assert(false);
    }
    catch (const RequestError &e)
    {
        cr_assert_eq(e.status, HTTP_VERSION_NOT_SUPPORTED);
    }
}

Test(request_line, http_version_update_required)
{
    std::string s = "GET /hello.htm HTTP/1.0";
    try
    {
        Request::parse_request_line(s);
        cr_assert(false);
    }
    catch (const RequestError &e)
    {
        cr_assert_eq(e.status, UPGRADE_REQUIRED);
    }
}

Test(request_line, http_0w0)
{
    std::string s = "GET /hello.htm HTTP/0w0";
    try
    {
        Request::parse_request_line(s);
        cr_assert(false);
    }
    catch (const RequestError &e)
    {
        cr_assert_eq(e.status, BAD_REQUEST);
    }
}

Test(request_header, success)
{
    std::string s = "GET /hello.htm HTTP/1.1";
    auto req = Request::parse_request_line(s);

    std::string s2 = "User-Agent: Mozilla/4.0";

    req->parse_header(s2);
    cr_assert(req->headers["User-Agent"].compare("Mozilla/4.0") == 0);
}

Test(request_header, optional_whitespace)
{
    std::string s = "GET /hello.htm HTTP/1.1";
    auto req = Request::parse_request_line(s);

    std::string s2 = "User-Agent: \t\t    Mozilla/4.0      ";

    req->parse_header(s2);
    cr_assert(req->headers["User-Agent"].compare("Mozilla/4.0") == 0);
}

Test(request_header, bad_whitespace)
{
    std::string s = "GET /hello.htm HTTP/1.1";
    auto req = Request::parse_request_line(s);

    std::string s2 = "User-Agent : Mozilla/4.0";

    try
    {
        req->parse_header(s2);
    }
    catch (const RequestError &e)
    {
        cr_assert_eq(e.status, BAD_REQUEST);
    }
}

Test(host_header, server_name)
{
    std::string s = "GET /hello.htm HTTP/1.1";
    auto req = Request::parse_request_line(s);

    std::string s2 = "Host: example.com";
    req->parse_header(s2);

    cr_assert(req->headers["Host"].compare("example.com") == 0);
}

Test(host_header, server_name_port)
{
    std::string s = "GET /hello.htm HTTP/1.1";
    auto req = Request::parse_request_line(s);

    std::string s2 = "Host: example.com:80";
    req->parse_header(s2);

    cr_assert(req->headers["Host"].compare("example.com:80") == 0);
}

Test(host_header, ip_port)
{
    std::string s = "GET /hello.htm HTTP/1.1";
    auto req = Request::parse_request_line(s);

    std::string s2 = "Host: 0.0.0.0:80";
    req->parse_header(s2);

    cr_assert(req->headers["Host"].compare("0.0.0.0:80") == 0);
}

Test(host_header_fail, port)
{
    std::string s = "GET /hello.htm HTTP/1.1";
    auto req = Request::parse_request_line(s);

    std::string s2 = "Host: :80";

    try
    {
        req->parse_header(s2);
    }
    catch (const RequestError &e)
    {
        cr_assert_eq(e.status, BAD_REQUEST);
    }
}

Test(host_header_fail, empty)
{
    std::string s = "GET /hello.htm HTTP/1.1";
    auto req = Request::parse_request_line(s);

    std::string s2 = "Host:";

    try
    {
        req->parse_header(s2);
    }
    catch (const RequestError &e)
    {
        cr_assert_eq(e.status, BAD_REQUEST);
    }
}

Test(host_header_fail, server_name_ip)
{
    std::string s = "GET /hello.htm HTTP/1.1";
    auto req = Request::parse_request_line(s);

    std::string s2 = "Host: example.com:0.0.0.0";

    try
    {
        req->parse_header(s2);
    }
    catch (const RequestError &e)
    {
        cr_assert_eq(e.status, BAD_REQUEST);
    }
}

Test(request_parse, one_shot)
{
    auto conn = std::make_shared<Connection>(nullptr, "0.0.0.0", 8000);

    conn->raw = "GET /hello.htm HTTP/1.1\r\nUser-Agent: Mozilla/4.0\r\nHost: "
                "www.tutorialspoint.com\r\n\r\n";

    cr_assert(Request::parse(conn) != nullptr);
    cr_assert(conn->req == nullptr);
}

Test(request_parse, content_length_too_big)
{
    auto conn = std::make_shared<Connection>(nullptr, "0.0.0.0", 8000);

    conn->raw = "GET /hello.htm HTTP/1.1\r\nUser-Agent: Mozilla/4.0\r\nHost: "
                "www.tutorialspoint.com\r\nContent-Length: 10\r\n\r\n";

    cr_assert(Request::parse(conn) == nullptr);
    cr_assert(conn->req != nullptr);
}

Test(request_parse, full)
{
    auto conn = std::make_shared<Connection>(nullptr, "0.0.0.0", 8000);

    conn->raw = "GET /hel";

    cr_assert(Request::parse(conn) == nullptr);
    cr_assert(conn->req == nullptr);

    conn->raw += "lo.htm HTTP/1.1\r\n";

    cr_assert(Request::parse(conn) == nullptr);
    cr_assert(conn->req != nullptr);

    conn->raw += "User-Agent: Mozilla/4.0\r\nHost: www.tutorialspoint.com";

    cr_assert(Request::parse(conn) == nullptr);
    cr_assert(conn->req != nullptr);

    conn->raw += "\r\n";

    cr_assert(Request::parse(conn) == nullptr);
    cr_assert(conn->req != nullptr);

    conn->raw += "\r\n";

    auto r = Request::parse(conn);

    cr_assert(conn->req == nullptr);
    cr_assert(r != nullptr);

    auto req = r;

    cr_assert(req->headers["User-Agent"].compare("Mozilla/4.0") == 0);
    cr_assert(req->headers["Host"].compare("www.tutorialspoint.com") == 0);
    cr_assert(req->method == METHOD::GET);
    cr_assert(req->target.compare("/hello.htm") == 0);
    cr_assert(req->http_version.compare("1.1") == 0);
}

Test(request_parse_double, full)
{
    auto conn = std::make_shared<Connection>(nullptr, "0.0.0.0", 8000);

    cr_assert(Request::parse(conn) == nullptr);
    cr_assert(conn->req == nullptr);

    conn->raw += "GET / HTTP/1.1\r\nHost: localhost\r\n\r\nGET / "
                 "HTTP/1.1\r\nHost: example.com\r\n\r\n";
    auto r1 = Request::parse(conn);
    cr_assert(conn->req == nullptr);
    cr_assert(r1 != nullptr);
    auto req1 = r1;
    cr_assert(req1->headers["Host"].compare("localhost") == 0);
    cr_assert(req1->method == METHOD::GET);
    cr_assert(req1->http_version.compare("1.1") == 0);

    auto r2 = Request::parse(conn);
    cr_assert(conn->req == nullptr);
    cr_assert(r2 != nullptr);

    auto req2 = r2;
    cr_assert(req2->headers["Host"].compare("example.com") == 0);
    cr_assert(req2->method == METHOD::GET);
    cr_assert(req2->http_version.compare("1.1") == 0);
}
