#ifndef RECEIVER_SERVER_H
#define RECEIVER_SERVER_H

#include <stdbool.h>
#include <stdint.h>

#include "net.h"

struct receiver_server {
    socket_t socket;
    uint16_t port;
};


// init default values
void
receiver_server_init(struct receiver_server *server, uint16_t port);

// block until the communication with the server is established
bool
receiver_server_connect(struct receiver_server *server);

// disconnect and kill the server process
void
receiver_server_disconnect(struct receiver_server *server);

#endif
