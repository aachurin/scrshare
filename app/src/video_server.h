#ifndef VIDEO_SERVER_H
#define VIDEO_SERVER_H

#include <stdbool.h>
#include <stdint.h>

#include "net.h"

struct video_server {
    socket_t socket;
    uint16_t port;
};


// init default values
void
video_server_init(struct video_server *server, uint16_t port);

// block until the communication with the server is established
bool
video_server_connect(struct video_server *server);

// disconnect and kill the server process
void
video_server_disconnect(struct video_server *server);

#endif
