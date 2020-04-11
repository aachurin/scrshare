#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include <stdint.h>

#include "net.h"

struct server {
    socket_t video_socket;
    uint16_t local_port;
};


// init default values
void
server_init(struct server *server, uint16_t local_port);

// block until the communication with the server is established
bool
server_connect(struct server *server);

// disconnect and kill the server process
void
server_disconnect(struct server *server);

#endif
