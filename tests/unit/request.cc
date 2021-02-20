#include "request/request.hh"

#include <criterion/criterion.h>
#include <exception>
#include <iostream>

#include "config/config.hh"
#include "error/init-error.hh"

Test(request_line, success)
{
    std::string s = "GET /hello.htm HTTP/1.1";
    auto req = http::Request::parse_request_line(s);
    cr_assert(req != std::nullopt);

    cr_assert(req.value()->method == http::METHOD::GET);
    cr_assert(req.value()->uri.compare("/hello.htm") == 0);
    cr_assert(req.value()->http_version.compare("1.1") == 0);
}

Test(request_line, success_post)
{
    std::string s = "POST /hello.htm HTTP/1.1";
    auto req = http::Request::parse_request_line(s);
    cr_assert(req != std::nullopt);

    cr_assert(req.value()->method == http::METHOD::POST);
    cr_assert(req.value()->uri.compare("/hello.htm") == 0);
    cr_assert(req.value()->http_version.compare("1.1") == 0);
}

Test(request_line, method_fail)
{
    std::string s = "PTnDETESTDEMERDE /hello.htm HTTP/1.1";
    cr_assert(http::Request::parse_request_line(s) == std::nullopt);
}

Test(request_line, http_version_fail)
{
    std::string s = "GET /hello.htm hachetétépé/unpointun";
    cr_assert(http::Request::parse_request_line(s) == std::nullopt);
}

Test(request_line, http_version_unsupported)
{
    std::string s = "GET /hello.htm HTTP/2.0";
    cr_assert(http::Request::parse_request_line(s) == std::nullopt);
}

Test(request_header, success)
{
    std::string s = "GET /hello.htm HTTP/1.1";
    auto req = http::Request::parse_request_line(s);

    std::string s2 = "User-Agent: Mozilla/4.0";
    cr_assert(http::Request::parse_request_header(s2, req.value()));
    cr_assert(req.value()->headers["User-Agent"].compare("Mozilla/4.0") == 0);
}
