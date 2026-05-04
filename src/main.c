#include <stdio.h>

#include "log.h"
#include "role.h"
#include "server.h"
#include "client.h"

#define DEFAULT_PORT 6666

static volatile sig_atomic_t stop = 0;

void handle_sig(int sig);
int run_server(const char *addr, uint16_t port);
int run_client(const char *addr, uint16_t port);

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("./tp <role> <addr> <port [optional]>\n");
        return -1;
    }

    struct sigaction sa;
    sa.sa_handler = handle_sig;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // no SA_RESTART so recvfrom returns EINTR
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    const char *role_str = argv[1];
    ROLE role = string_to_role(role_str);
    if (role == UNKNOWN) {
        ERROR("invalid role");
        return -1;
    }

    const char *addr = argv[2];
    uint16_t port = (argc >= 4) ? (uint16_t)atoi(argv[3]) : (uint16_t)DEFAULT_PORT;

    switch (role) {
        case SERVER: {
            if (run_server(addr, port) < 0)
                exit(1);

            break;
        }
        case CLIENT: {
            if (run_client(addr, port) < 0)
                exit(1);

            break;
        }
        default: {
            ERROR("invalid role");
            exit(1);
        }
    }

    return 0;
}

void handle_sig(int sig) {
    (void)sig;
    stop = 1;
}

int run_server(const char *addr, uint16_t port) {
    Server server;
    if (open_server(addr, port, &server) < 0) {
        ERROR("failed to open server");
        return -1;
    }

    if (handle_server(&server, &stop) < 0) {
        close_server(&server);
        return -1;
    }

    close_server(&server);
    return 0;
}

int run_client(const char *addr, uint16_t port) {
    Client client;
    if (open_client(addr, port, &client) < 0) {
        ERROR("failed to open server");
        return -1;
    }

    if (handle_client(&client, &stop) < 0) {
        close_client(&client);
        return -1;
    }

    close_client(&client);
    return 0;
}
