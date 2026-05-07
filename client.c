#include "client.h"

#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <termios.h>

#include "device.h"
#include "log.h"
#include "packet.h"

static struct termios g_saved_tio;
static int g_tio_saved = 0;

static void mute_terminal(void) {
    if (!isatty(STDIN_FILENO)) return;
    if (tcgetattr(STDIN_FILENO, &g_saved_tio) != 0) return;
    struct termios raw = g_saved_tio;
    raw.c_lflag &= ~(ECHO | ECHONL | ICANON);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0)
        g_tio_saved = 1;
}

static void restore_terminal(void) {
    if (!g_tio_saved) return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_saved_tio);
    g_tio_saved = 0;
}

int open_client(const char *addr, uint16_t port, Client *client, int keyboard, int mouse) {
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

    memset(client, 0, sizeof(*client));
    client->fd = fd;
    client->sa = sa;
    client->keyboard_fd = keyboard;
    client->mouse_fd = mouse;

    LOG("Opened new client (%s:%u)", addr, port);

    return 0;
}

static int send_all(int fd, const uint8_t *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, buf + sent, len - sent, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
}

int handle_client(Client *client, volatile sig_atomic_t *stop) {
    if (!client)
        return -1;

    if (client->keyboard_fd < 0) {
        ERROR("no keyboard fd to read from");
        return -1;
    }

    if (ioctl(client->keyboard_fd, EVIOCGRAB, 1) < 0) {
        ERROR("failed to grab keyboard");
        return -1;
    }

    mute_terminal();

    int rc = 0;
    struct pollfd pfd = {
        .fd = client->keyboard_fd,
        .events = POLLIN,
    };

    // disconnect hotkey: LCtrl + RCtrl + LAlt + RAlt all held
    int held_lctrl = 0, held_rctrl = 0, held_lalt = 0, held_ralt = 0;

    while (!*stop) {
        int r = poll(&pfd, 1, 500);
        if (r < 0) {
            if (errno == EINTR) continue;
            ERROR("poll failed");
            rc = -1;
            break;
        }
        if (r == 0) continue;
        if (!(pfd.revents & POLLIN)) continue;

        struct input_event ev;
        ssize_t n = read(client->keyboard_fd, &ev, sizeof(ev));
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                continue;
            ERROR("keyboard read failed");
            rc = -1;
            break;
        }
        if (n != (ssize_t)sizeof(ev))
            continue;

        if (ev.type == EV_KEY) {
            int down = (ev.value != 0);
            switch (ev.code) {
                case KEY_LEFTCTRL:  held_lctrl = down; break;
                case KEY_RIGHTCTRL: held_rctrl = down; break;
                case KEY_LEFTALT:   held_lalt  = down; break;
                case KEY_RIGHTALT:  held_ralt  = down; break;
                default: break;
            }
            if (held_lctrl && held_rctrl && held_lalt && held_ralt) {
                ioctl(client->keyboard_fd, EVIOCGRAB, 0);
                LOG("disconnect hotkey pressed (L+R Ctrl + L+R Alt)");
                break;
            }
        }

        packet p = {
            .version = PROTOCOL_VERSION,
            .type = KEYBOARD,
            .event = ev.type,
            .code = ev.code,
            .value = ev.value
        };
        uint8_t buffer[PACKET_SIZE];
        encode_packet(&p, buffer);

        if (send_all(client->fd, buffer, PACKET_SIZE) < 0) {
            ERROR("failed to send data");
            rc = -1;
            break;
        }
    }

    ioctl(client->keyboard_fd, EVIOCGRAB, 0);
    restore_terminal();
    return rc;
}

void close_client(Client *client) {
    if (!client)
        return;

    restore_terminal();

    if (client->keyboard_fd >= 0)
        close(client->keyboard_fd);

    if (client->mouse_fd >= 0)
        close(client->mouse_fd);

    if (client->fd >= 0)
        close(client->fd);

    LOG("Closed client");
}
