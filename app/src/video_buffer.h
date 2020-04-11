#ifndef VIDEO_BUFFER_H
#define VIDEO_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>

// forward declarations
typedef struct AVFrame AVFrame;

struct video_buffer {
    AVFrame *decoding_frame;
    AVFrame *rendering_frame;
    uint16_t video_render_interval;
    const char* video_buffer_name;
    uint32_t video_buffer_size;
    void *video_buffer;
    sem_t *video_lock;
    const char* video_lock_name;
    uintmax_t video_render_clock;
    uint32_t video_render_key;
};

bool
video_buffer_init(struct video_buffer *vb,
                  const char *video_buffer_name,
                  uint32_t video_buffer_size,
                  uint16_t video_render_interval);

void
video_buffer_destroy(struct video_buffer *vb);

// set the decoded frame as ready for rendering
// this function locks frames->mutex during its execution
// the output flag is set to report whether the previous frame has been skipped
void
video_buffer_offer_decoded_frame(struct video_buffer *vb);

#endif
