#include "video_buffer.h"

#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>

#include "log.h"

bool
video_buffer_init(struct video_buffer *vb,
                  socket_t socket,
                  uint32_t video_frame_size,
                  uint16_t video_render_interval)
{
    memset(vb, 0, sizeof(struct video_buffer));

    vb->video_render_interval = video_render_interval;
    vb->video_render_clock = 0;
    vb->video_render_key = 0;
    vb->video_frame_size = video_frame_size;
    vb->socket = socket;

    if (!(vb->decoding_frame = av_frame_alloc())) {
        LOGC("av_frame_alloc failed");
        video_buffer_destroy(vb);
        return false;
    }

    if (!(vb->rendering_frame = av_frame_alloc())) {
        LOGC("av_frame_alloc failed");
        video_buffer_destroy(vb);
        return false;
    }


    LOGI("video frame size: %u", video_frame_size);

    return true;
}

void
video_buffer_destroy(struct video_buffer *vb) {
//    if (vb->video_buffer && vb->video_buffer != MAP_FAILED) {
//        munmap(vb->video_buffer, vb->video_buffer_size);
//    }
    if (vb->rendering_frame) {
        av_frame_free(&vb->rendering_frame);
    }
    if (vb->decoding_frame) {
        av_frame_free(&vb->decoding_frame);
    }
    memset(vb, 0, sizeof(struct video_buffer));
}

static void
video_buffer_swap_frames(struct video_buffer *vb) {
    AVFrame *tmp = vb->decoding_frame;
    vb->decoding_frame = vb->rendering_frame;
    vb->rendering_frame = tmp;
}

static uintmax_t
get_current_time ()
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    uintmax_t ms = spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
    return ms;
}

void
video_buffer_offer_decoded_frame(struct video_buffer *vb) {
    uint32_t buffer[3];
    video_buffer_swap_frames(vb);
    uintmax_t cur_time = get_current_time();
    uintmax_t next_render_time = vb->video_render_clock + vb->video_render_interval;
    if (next_render_time <= cur_time) {
        AVFrame *frame = vb->rendering_frame;
        uint32_t width = frame->linesize[0];
        uint32_t height = frame->height;
        uint32_t size = width * height;
        if (size > vb->video_frame_size) {
            return;
        }
        buffer[0] = size;
        buffer[1] = width;
        buffer[2] = height;
        net_send_all(vb->socket, &buffer, sizeof(buffer));
        net_send_all(vb->socket, frame->data[0], size);
        vb->video_render_clock = cur_time;
    }
}
