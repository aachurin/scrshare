#include "server.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "log.h"
#include "net.h"

#define IPV4_LOCALHOST 0x7F000001

static socket_t
connect_and_read_byte(uint16_t port) {
    socket_t socket = net_connect(IPV4_LOCALHOST, port);
    if (socket == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }

    LOGD("connect_and_read_byte: net_connect ok");

    char byte;
    // the connection may succeed even if the server behind the "adb tunnel"
    // is not listening, so read one byte to detect a working connection
    if (net_recv(socket, &byte, 1) != 1) {
        // the server is not listening yet behind the adb tunnel
        LOGD("connect_and_read_byte: net_recv failed");
        net_close(socket);
        return INVALID_SOCKET;
    }

    LOGD("connect_and_read_byte: success");
    return socket;
}

static socket_t
connect_to_server(uint16_t port, uint32_t attempts) {
    do {
        LOGD("remaining connection attempts: %d", (int) attempts);
        socket_t socket = connect_and_read_byte(port);
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
server_init(struct server *server, uint16_t local_port) {
    server->video_socket = INVALID_SOCKET;
    server->local_port = local_port;
}


bool
server_connect(struct server *server) {
    uint32_t attempts = 10;
    server->video_socket = connect_to_server(server->local_port, attempts);
    if (server->video_socket == INVALID_SOCKET) {
        return false;
    }
    return true;
}

void
server_disconnect(struct server *server) {
    if (server->video_socket != INVALID_SOCKET) {
        close_socket(&server->video_socket);
    }
}
