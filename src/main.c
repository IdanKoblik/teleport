#include <stdio.h>
#include <string.h>

#include "log.h"
#include "server.h"

#define DEFAULT_PORT 6666

int handle_server(const char *addr, uint16_t port);

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("./tp <type> <addr> <port [optional]>\n");
        return -1;
    }

    const char *conn_type = argv[1];
    const char *addr = argv[2];
    uint16_t port = (argc >= 4) ? (uint16_t)atoi(argv[3]) : (uint16_t)DEFAULT_PORT;

    if (!strcmp(conn_type, "server")) {
        if (handle_server(addr, port) < 0)
            exit(1);

        return 0;
    }

    return 0;
}

int handle_server(const char *addr, uint16_t port) {
    Server server;
    if (open_server(addr, port, &server) < 0) {
        ERROR("failed to open server\n");
        return -1;
    }

    close_server(&server);
    return 0;
}
