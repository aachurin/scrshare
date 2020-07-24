#include "scrshare.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libavformat/avformat.h>
#include <sys/time.h>

#include "common.h"
#include "compat.h"
#include "decoder.h"
#include "device.h"
#include "log.h"
#include "net.h"
#include "video_server.h"
#include "receiver_server.h"
#include "stream.h"
#include "video_buffer.h"

struct video_buffer video_buffer;
void* default_sigint_handler;

static void
before_exit()
{
    LOGI("close video buffer before exit.")
    video_buffer_destroy(&video_buffer);
}

bool
scrshare(uint16_t video_server_port, uint16_t receiver_server_port, uint16_t video_render_interval) {
    struct video_server video_server;
    struct receiver_server receiver_server;
    struct stream stream;
    struct decoder decoder;

    bool ret = false;
    bool video_buffer_initialized = false;

    video_server_init(&video_server, video_server_port);

    LOGI("connect to video server");

    if (!video_server_connect(&video_server)) {
        goto end;
    }

    LOGC("@video_server_connected");

    receiver_server_init(&receiver_server, receiver_server_port);

    LOGI("connect to receiver");

    if (!receiver_server_connect(&receiver_server)) {
        goto end;
    }

    LOGC("@receiver_server_connected");

    char device_name[DEVICE_NAME_FIELD_LENGTH];
    struct size frame_size;

    // screenrecord does not send frames when the screen content does not
    // change therefore, we transmit the screen size before the video stream,
    // to be able to init the window immediately
    if (!device_read_info(video_server.socket, device_name, &frame_size)) {
        goto end;
    }

    LOGI("Device: %s", device_name)
    LOGC("@screen %d, %d", frame_size.width, frame_size.height);

    uint32_t video_frame_size = frame_size.width * frame_size.height * 2;

    if (!video_buffer_init(&video_buffer,
                           receiver_server.socket,
                           video_frame_size,
                           video_render_interval)) {
        goto end;
    }

    video_buffer_initialized = true;
    atexit(before_exit);
    decoder_init(&decoder, &video_buffer);

    LOGI("start stream");
    stream_init(&stream, video_server.socket, &decoder);
    ret = stream_start(&stream);

end:
    // shutdown the sockets and kill the server
    video_server_disconnect(&video_server);
    receiver_server_disconnect(&receiver_server);

    // now that the sockets are shutdown, the stream and controller are
    // interrupted, we can join them
    if (video_buffer_initialized) {
        video_buffer_destroy(&video_buffer);
    }

    return ret;
}
