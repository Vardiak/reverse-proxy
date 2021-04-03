#include <criterion/criterion.h>
#include <exception>
#include <iostream>

#include "config/config.hh"
#include "error/init-error.hh"
#include "error/request-error.hh"
#include "request/request.hh"
#include "request/types.hh"
#include "vhost/dispatcher.hh"
#include "vhost/vhost-factory.hh"

using namespace http;

Test(response_parse_double, full)
{
    auto conn = std::make_shared<Connection>(nullptr, "0.0.0.0", 8000);

    cr_assert(Request::parse(conn) == nullptr);
    cr_assert(conn->req == nullptr);

    conn->raw +=
        "GET /workspaces/sws/tests/static/index.html HTTP/1.1\r\nHost: "
        "localhost\r\n\r\nHEAD /workspaces/sws/tests/static/index.html "
        "HTTP/1.1\r\nHost: localhost\r\n\r\n";

    auto r1 = Request::parse(conn);
    auto req1 = r1;

    size_t file_size =
        std::filesystem::file_size("/workspaces/sws/tests/static/index.html");

    auto response = std::make_shared<Response>(*req1, OK);
    cr_assert(response->headers["Connection"].compare("keep-alive") == 0);
    cr_assert(
        response->headers["Content-Length"].compare(std::to_string(file_size))
        == 0);
    cr_assert(response->content_length == file_size);
    cr_assert(response->is_file);
    cr_assert(response->status == OK);
    cr_assert(response->body.compare("/workspaces/sws/tests/static/index.html")
              == 0);

    auto r2 = Request::parse(conn);
    auto req2 = r2;

    auto response2 = std::make_shared<Response>(*req2, OK);
    cr_assert(response2->headers["Connection"].compare("keep-alive") == 0);
    cr_assert(response2->headers["Content-Length"].compare("0") == 0);
    cr_assert(response2->content_length == 0);
    cr_assert(!response2->is_file);
    cr_assert(response2->status == OK);
    cr_assert(response2->body.compare("") == 0);
}

Test(response_error_parse, full)
{
    std::string str = "Not Found";

    auto response = std::make_shared<Response>(NOT_FOUND);
    cr_assert(response->headers["Connection"].compare("close") == 0);
    cr_assert(response->headers["Content-Length"].compare("0") == 0);
    cr_assert(response->content_length == 0);
    cr_assert(!response->is_file);
    cr_assert(response->status == NOT_FOUND);
    cr_assert(response->body.compare("") == 0);
}
