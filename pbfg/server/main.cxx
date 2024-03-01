#include <iostream>
#include <limits>
#include <cstdlib> // ::atoll()

#include "server.hpp"

int main(int argc, char **argv) {
    NETINIT;

    bool show_usage = true;
    long long server_port{};

    if (argc >= 2) {
        server_port = ::atoll(argv[1]);
        if (server_port < 0 || server_port >= std::numeric_limits<unsigned>::max()) {
            show_usage = true;
        } else {
            show_usage = false;
        }
    }

    if (show_usage) {
        std::cout << "USAGE: " << argv[0]
                  << " server_port"
                  << std::endl;
        return 1;
    }

    Server server((unsigned) server_port);
    server.loop();
    return 0;
}
