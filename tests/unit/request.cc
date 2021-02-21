#include "request/request.hh"

#include <criterion/criterion.h>
#include <exception>
#include <iostream>

#include "config/config.hh"
#include "error/init-error.hh"
#include "error/request-error.hh"

using namespace http;

Test(request_line, success)
{
    std::string s = "GET /hello.htm HTTP/1.1";
    auto req = Request::parse_request_line(s);

    cr_assert(req->method == METHOD::GET);
    cr_assert(req->uri.compare("/hello.htm") == 0);
    cr_assert(req->http_version.compare("1.1") == 0);
}

Test(request_line, success_post)
{
    std::string s = "POST /hello.htm HTTP/1.1";
    auto req = Request::parse_request_line(s);

    cr_assert(req->method == METHOD::POST);
    cr_assert(req->uri.compare("/hello.htm") == 0);
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

Test(request_header, success)
{
    std::string s = "GET /hello.htm HTTP/1.1";
    auto req = Request::parse_request_line(s);

    std::string s2 = "User-Agent: Mozilla/4.0";

    Request::parse_request_header(s2, req);
    cr_assert(req->headers["User-Agent"].compare("Mozilla/4.0") == 0);
}

Test(request_parse, one_shot)
{
    auto conn = std::make_shared<Connection>();

    conn->raw = "GET /hello.htm HTTP/1.1\r\nUser-Agent: Mozilla/4.0\r\nHost: "
                "www.tutorialspoint.com\r\n\r\n";

    cr_assert(Request::parse(conn) != std::nullopt);
    cr_assert(conn->req == std::nullopt);
}

Test(request_parse, full)
{
    auto conn = std::make_shared<Connection>();

    conn->raw = "GET /hel";

    cr_assert(Request::parse(conn) == std::nullopt);
    cr_assert(conn->req == std::nullopt);

    conn->raw += "lo.htm HTTP/1.1\r\n";

    cr_assert(Request::parse(conn) == std::nullopt);
    cr_assert(conn->req != std::nullopt);

    conn->raw += "User-Agent: Mozilla/4.0\r\nHost: www.tutorialspoint.com";

    cr_assert(Request::parse(conn) == std::nullopt);
    cr_assert(conn->req != std::nullopt);

    conn->raw += "\r\n";

    cr_assert(Request::parse(conn) == std::nullopt);
    cr_assert(conn->req != std::nullopt);

    conn->raw += "\r\n";

    cr_assert(Request::parse(conn) != std::nullopt);
    cr_assert(conn->req == std::nullopt);
}
