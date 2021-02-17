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
