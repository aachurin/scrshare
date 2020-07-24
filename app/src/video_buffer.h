#ifndef VIDEO_BUFFER_H
#define VIDEO_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include "net.h"

// forward declarations
typedef struct AVFrame AVFrame;

struct video_buffer {
    AVFrame *decoding_frame;
    AVFrame *rendering_frame;
    uint16_t video_render_interval;
    uintmax_t video_render_clock;
    uint32_t video_render_key;
    uint32_t video_frame_size;
    socket_t socket;
};

bool
video_buffer_init(struct video_buffer *vb,
                  socket_t socket,
                  uint32_t video_frame_size,
                  uint16_t video_render_interval);

void
video_buffer_destroy(struct video_buffer *vb);

// set the decoded frame as ready for rendering
// this function locks frames->mutex during its execution
// the output flag is set to report whether the previous frame has been skipped
void
video_buffer_offer_decoded_frame(struct video_buffer *vb);

#endif
