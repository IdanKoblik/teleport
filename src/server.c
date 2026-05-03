#include "server.h"

#include <sys/socket.h>
#include <arpa/inet.h>

#include "log.h"

int open_server(const char *addr, uint16_t port, Server *server) {
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERROR("failed to open server socket\n");
        return -1;
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET; // IPv4
    sa.sin_port = htons(port);

    inet_pton(AF_INET, addr, &(sa.sin_addr));

    if (bind(fd, (const struct sockaddr *)&sa, sizeof(sa)) < 0) {
        ERROR("server bind failed");
        close(fd);
        return -1;
    }

    memset(server, 0, sizeof(*server));
    server->fd = fd;
    server->addr = sa;
    LOG("Opened new server (%s:%u)\n", addr, port);

    return 0;
}

void close_server(Server *server) {
    if (!server)
        return;

    LOG("Closed server\n");
}
