#include "client.h"

#include <unistd.h>
#include <arpa/inet.h>

#include "log.h"

int open_client(const char *addr, uint16_t port, Client *client) {
    if (!client)
        return -1;

    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERROR("socket creation failed");
        return -1;
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET; // IPv4
    sa.sin_port = htons(port);

    if (inet_pton(AF_INET, addr, &(sa.sin_addr)) != 1) {
        ERROR("invalid server address");
        close(fd);
        return -1;
    }

    if (connect(fd, (const struct sockaddr *)&sa, sizeof(sa)) < 0) {
        ERROR("cannot connect to the server");
        close(fd);
        return -1;
    }

    const char *msg = "hello";
    if (send(fd, msg, strlen(msg), 0) < 0) {
        ERROR("failed to send message to the server");
        close(fd);
        return -1;
    }

    memset(client, 0, sizeof(*client));
    client->fd = fd;
    client->sa = sa;

    LOG("Opened new client (%s:%u)", addr, port);

    return 0;
}

int handle_client(Client *client, volatile sig_atomic_t *stop) {
    if (!client)
        return -1;

    while (!*stop) {
    }

    return 0;
}

void close_client(Client *client) {
    if (!client)
        return;

    if (client->fd >= 0)
        close(client->fd);

    LOG("Closed client");
}
