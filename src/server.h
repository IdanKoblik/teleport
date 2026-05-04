#ifndef SERVER_H_
#define SERVER_H_

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

// TCP server
typedef struct Server {
    int fd;
    struct sockaddr_in addr;
} Server;

int open_server(const char *addr, uint16_t port, Server *server);
int handle_server(const Server *server, volatile sig_atomic_t *stop);
void close_server(Server *server);

#endif // SERVER_H_
