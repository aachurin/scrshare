#include "video_buffer.h"

#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>

#include "log.h"

bool
video_buffer_init(struct video_buffer *vb,
                  const char *video_buffer_name,
                  uint32_t video_buffer_size,
                  uint16_t video_render_interval)
{
    memset(vb, 0, sizeof(struct video_buffer));

    vb->video_render_interval = video_render_interval;
    vb->video_buffer_size = video_buffer_size;
    vb->video_render_clock = 0;
    vb->video_render_key = 0;

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

    int len = strlen(video_buffer_name);
    char *video_lock_name = malloc(len + 6);
    strcpy(video_lock_name, video_buffer_name);
    strcat(video_lock_name, ".lock");
    vb->video_lock_name = video_lock_name;

    if ((vb->video_lock = sem_open(video_lock_name, O_CREAT, 0666, 1)) == SEM_FAILED) {
        LOGC("sem_open failed: %s (%d)", strerror(errno), errno);
        video_buffer_destroy(vb);
        return false;
    }

    LOGI("video buffer: %s (size: %u)", video_buffer_name, video_buffer_size);

    shm_unlink(video_buffer_name);
    int fd = shm_open(video_buffer_name, O_CREAT | O_TRUNC | O_RDWR, 0666);
    if (fd == -1) {
        LOGC("shm_open failed: %s (%d)", strerror(errno), errno);
        video_buffer_destroy(vb);
        return false;
    }

    vb->video_buffer_name = video_buffer_name;

    if (ftruncate(fd, video_buffer_size)) {
        LOGC("ftruncate failed: %s, %d", strerror(errno), errno);
        close(fd);
        video_buffer_destroy(vb);
        return false;
    }

    vb->video_buffer = mmap(0, video_buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (vb->video_buffer == MAP_FAILED) {
        LOGC("mmap failed: %s, %d", strerror(errno), errno);
        close(fd);
        video_buffer_destroy(vb);
        return false;
    }

    close(fd);

    return true;
}

void
video_buffer_destroy(struct video_buffer *vb) {
    if (vb->video_buffer && vb->video_buffer != MAP_FAILED) {
        munmap(vb->video_buffer, vb->video_buffer_size);
    }
    if (vb->video_buffer_name) {
        shm_unlink(vb->video_buffer_name);
    }
    if (vb->video_lock && vb->video_lock != SEM_FAILED) {
        sem_unlink(vb->video_lock_name);
    }
    if (vb->video_lock_name) {
        free((void*)(vb->video_lock_name));
    }
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
    video_buffer_swap_frames(vb);
    uintmax_t cur_time = get_current_time();
    uintmax_t next_render_time = vb->video_render_clock + vb->video_render_interval;
    if (next_render_time <= cur_time) {
        if (!sem_trywait(vb->video_lock)) {
            AVFrame *frame = vb->rendering_frame;
            uint32_t width = frame->linesize[0];
            uint32_t height = frame->height;
            vb->video_render_key += 1;
            uint32_t* buff = (uint32_t*)(vb->video_buffer);
            buff[0] = vb->video_render_key;
            buff[1] = width;
            buff[2] = height;
            uint32_t offset = sizeof(vb->video_render_key) + sizeof(width) + sizeof(height);
            memcpy(&(vb->video_buffer[offset]), frame->data[0], width * height);
            sem_post(vb->video_lock);
            vb->video_render_clock = cur_time;
        }
    }
}
