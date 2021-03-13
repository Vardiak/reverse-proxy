#include "config/config.hh"

#include <criterion/criterion.h>
#include <exception>
#include <iostream>

#include "error/init-error.hh"

Test(config, subject)
{
    http::parse_configuration("tests/configs/config_subject.json");
}

Test(config, ip_not_string)
{
    try
    {
        http::parse_configuration("tests/configs/config_ip_not_string.json");
        cr_assert(false);
    }
    catch (const http::InitializationError &e)
    {
        cr_assert(strcmp(e.what(), "Initialization error: invalid ip") == 0);
    }
}

Test(config, port_not_int)
{
    try
    {
        http::parse_configuration("tests/configs/config_port_not_int.json");
        cr_assert(false);
    }
    catch (const http::InitializationError &e)
    {
        cr_assert(strcmp(e.what(), "Initialization error: invalid port") == 0);
    }
}

Test(config, auth_basic_error)
{
    try
    {
        http::parse_configuration("tests/configs/config_auth_basic_error.json");
        cr_assert(false);
    }
    catch (const http::InitializationError &e)
    {
        cr_assert(strcmp(e.what(),
                         "Initialization error: auth_basic and "
                         "auth_basic_users must be defined together")
                  == 0);
    }
}

Test(config, auth_basic_not_string)
{
    try
    {
        http::parse_configuration("tests/configs/config_auth_not_string.json");
        cr_assert(false);
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << std::endl;
        cr_assert(strcmp(e.what(),
                         "[json.exception.type_error.302] type must be string, "
                         "but is number")
                  == 0);
    }
}

Test(config, auth_basic_column_missing)
{
    try
    {
        http::parse_configuration(
            "tests/configs/config_auth_column_missing.json");
        cr_assert(false);
    }
    catch (const http::InitializationError &e)
    {
        std::cout << e.what() << std::endl;
        cr_assert(strcmp(e.what(),
                         "Initialization error: Basic users' syntax must be "
                         "username:password")
                  == 0);
    }
}

Test(config, auth_basic_users_error)
{
    try
    {
        http::parse_configuration(
            "tests/configs/config_auth_basic_users_error.json");
        cr_assert(false);
    }
    catch (const http::InitializationError &e)
    {
        cr_assert(strcmp(e.what(),
                         "Initialization error: auth_basic and "
                         "auth_basic_users must be defined together")
                  == 0);
    }
}

Test(config, ssl_key_error)
{
    try
    {
        http::parse_configuration("tests/configs/config_ssl_key_error.json");
        cr_assert(false);
    }
    catch (const http::InitializationError &e)
    {
        cr_assert(strcmp(e.what(),
                         "Initialization error: ssl_cert and ssl_key must be "
                         "defined together")
                  == 0);
    }
}

Test(config, ssl_cert_error)
{
    try
    {
        http::parse_configuration("tests/configs/config_ssl_cert_error.json");
        cr_assert(false);
    }
    catch (const http::InitializationError &e)
    {
        cr_assert(strcmp(e.what(),
                         "Initialization error: ssl_cert and ssl_key must be "
                         "defined together")
                  == 0);
    }
}

Test(config, multiple_default_vhosts)
{
    try
    {
        http::parse_configuration(
            "tests/configs/config_several_default_vhost.json");
        cr_assert(false);
    }
    catch (const http::InitializationError &e)
    {
        cr_assert(
            strcmp(
                e.what(),
                "Initialization error: Default vhost must be unique and valid")
            == 0);
    }
}
