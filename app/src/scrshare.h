#ifndef SCRSHARE_H
#define SCRSHARE_H

#include <stdbool.h>
#include <stdint.h>

bool
scrshare(uint16_t port, const char* video_buffer_name, uint16_t video_render_interval);

#endif
