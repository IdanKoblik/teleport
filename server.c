#include "server.h"

#include <sys/socket.h>
#include <arpa/inet.h>

#include "device.h"
#include "log.h"
#include "packet.h"

int open_server(const char *addr, uint16_t port, Server *server) {
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERROR("failed to open server socket");
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

    if (listen(fd, 1) < 0) {
        ERROR("server listen failed");
        close(fd);
        return -1;
    }

    memset(server, 0, sizeof(*server));
    server->fd = fd;
    server->addr = sa;

    int virtual_device = create_vdevice();
    if (virtual_device < 0) {
        ERROR("failed to create virtual device");
        close(fd);
        return -1;
    }

    server->virtual_device = virtual_device;
    LOG("Opened new server (%s:%u)", addr, port);

    return 0;
}

int handle_server(const Server *server, volatile sig_atomic_t *stop) {
    if (!server)
        return -1;

    struct timeval tv = {
       .tv_sec = 1,
       .tv_usec = 0
    };

    if (setsockopt(server->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        ERROR("setsockopt SO_RECTIMEO failed");
        return -1;
    }

    while (!*stop) {
        struct sockaddr_in peer;
        socklen_t peer_len = sizeof(peer);
        int client_fd = accept(server->fd, (struct sockaddr *)&peer, &peer_len);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                continue;
            ERROR("accept failed");
            return -1;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &peer.sin_addr, ip, sizeof(ip));
        LOG("Accepted connection from %s:%u", ip, ntohs(peer.sin_port));

        if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            ERROR("setsockopt SO_RCVTIMEO failed on client");
            close(client_fd);
            continue;
        }

        uint8_t buffer[PACKET_SIZE];
        size_t have = 0;

        while (!*stop) {
            ssize_t n = recv(client_fd, buffer + have, PACKET_SIZE - have, 0);
            if (n == 0) {
                LOG("Peer %s:%u disconnected", ip, ntohs(peer.sin_port));
                break;
            }
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                    continue;
                ERROR("recv failed");
                break;
            }

            have += (size_t)n;
            if (have < PACKET_SIZE)
                continue;

            packet p;
            decode_packet(buffer, &p);
            have = 0;

            if (p.version != PROTOCOL_VERSION) {
                WARN("dropping packet with unsupported version %u", p.version);
                continue;
            }

            if (server->virtual_device < 0)
                continue;

            emit_key(server->virtual_device, p.event, p.code, p.value);

            LOG("Received packet type=%u event=%u code=%u value=%d",
                p.type, p.event, p.code, p.value);
        }

        close(client_fd);
        LOG("Closed connection from %s:%u", ip, ntohs(peer.sin_port));
    }

    return 0;
}

void close_server(Server *server) {
    if (!server)
        return;

    if (server->fd >= 0)
        close(server->fd);

    if (server->virtual_device >= 0)
        close(server->virtual_device);

    LOG("Closed server");
}
