#include <iostream>

#include "config/config.hh"
#include "error/not-implemented.hh"
#include "misc/readiness/readiness.hh"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        // show usage
        std::cerr << "Usage: ./spider [path to server config]\n";
        return 1;
    }

    // bool dry_run = argc == 3 && argv[1][0] == '-' && argv[1][1] == 't';

    auto config = http::parse_configuration(argv[1]);

    misc::announce_spider_readiness(argv[0]);
    return 0;
}
