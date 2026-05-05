#ifndef CLIENT_H_
#define CLIENT_H_

#include <signal.h>
#include <netinet/in.h>

typedef struct Client {
    int fd;
    struct sockaddr_in sa;
} Client;

int open_client(const char *addr, uint16_t port, Client *client);
int handle_client(Client *client, volatile sig_atomic_t *stop);
void close_client(Client *client);

#endif // CLIENT_H_
