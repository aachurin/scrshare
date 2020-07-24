#include "receiver_server.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "log.h"
#include "net.h"

#define IPV4_LOCALHOST 0x7F000001

static socket_t
connect_port(uint16_t port) {
    socket_t socket = net_connect(IPV4_LOCALHOST, port);
    if (socket == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }

    LOGD("connect: net_connect ok");
    return socket;
}

static socket_t
connect_to_server(uint16_t port, uint32_t attempts) {
    do {
        LOGD("remaining connection attempts: %d", (int) attempts);
        socket_t socket = connect_port(port);
        if (socket != INVALID_SOCKET) {
            return socket;
        }
        if (attempts) {
            sleep(1);
        }
    } while (--attempts > 0);
    return INVALID_SOCKET;
}

static void
close_socket(socket_t *socket) {
    assert(*socket != INVALID_SOCKET);
    net_shutdown(*socket, SHUT_RDWR);
    if (!net_close(*socket)) {
        LOGW("could not close socket");
        return;
    }
    *socket = INVALID_SOCKET;
}

void
receiver_server_init(struct receiver_server *server, uint16_t port) {
    server->socket = INVALID_SOCKET;
    server->port = port;
}


bool
receiver_server_connect(struct receiver_server *server) {
    uint32_t attempts = 10;
    server->socket = connect_to_server(server->port, attempts);
    if (server->socket == INVALID_SOCKET) {
        return false;
    }
    return true;
}

void
receiver_server_disconnect(struct receiver_server *server) {
    if (server->socket != INVALID_SOCKET) {
        close_socket(&server->socket);
    }
}
