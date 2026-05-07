#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <fcntl.h>

#include "device.h"
#include "log.h"
#include "role.h"
#include "server.h"
#include "client.h"
#include "select.h"

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
    uint16_t port = (argc >= 4) ? (uint16_t)atoi(argv[3]) :
    (uint16_t)DEFAULT_PORT;

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
    size_t count = 0;
    device_entry *list = fetch_devices(KEYBOARD, &count);
    if (!list) {
        ERROR("no keyboard devices available");
        return -1;
    }

    int idx = select_keyboard(list, count);
    if (idx < 0) {
        free(list);
        return 0;
    }

    printf("selected: %s -> %s\n", list[idx].name, list[idx].devnode);

    int keyboard_fd = open(list[idx].devnode, O_RDONLY | O_NONBLOCK);
    free(list);
    if (keyboard_fd < 0) {
        ERROR("failed to open keyboard device (need read permission, e.g. input group or root)");
        return -1;
    }

    Client client;
    if (open_client(addr, port, &client, keyboard_fd, -1) < 0) {
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
