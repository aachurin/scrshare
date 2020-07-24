#ifndef SCRSHARE_H
#define SCRSHARE_H

#include <stdbool.h>
#include <stdint.h>

bool
scrshare(uint16_t video_server_port, uint16_t receiver_server_port, uint16_t video_render_interval);

#endif
