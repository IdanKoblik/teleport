#ifndef SERVER_H_
#define SERVER_H_

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// TCP server
typedef struct Server {
    int fd;
    struct sockaddr_in addr;
} Server;

int open_server(const char *addr, uint16_t port, Server *server);
void close_server(Server *server);

#endif // SERVER_H_
