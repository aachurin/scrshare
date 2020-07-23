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
#include "server.h"
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
scrshare(uint16_t port, const char* video_buffer_name, uint16_t video_render_interval) {
    struct server server;
    struct stream stream;
    struct decoder decoder;

    bool ret = false;
    bool video_buffer_initialized = false;

    server_init(&server, port);

    LOGI("connect to server");

    if (!server_connect(&server)) {
        goto end;
    }

    LOGC("@socket_connected");

    char device_name[DEVICE_NAME_FIELD_LENGTH];
    struct size frame_size;

    // screenrecord does not send frames when the screen content does not
    // change therefore, we transmit the screen size before the video stream,
    // to be able to init the window immediately
    if (!device_read_info(server.video_socket, device_name, &frame_size)) {
        goto end;
    }

    LOGI("Device: %s", device_name)
    LOGC("@screen %d, %d", frame_size.width, frame_size.height);

    uint32_t video_buffer_size = frame_size.width * frame_size.height * 2;

    if (!video_buffer_init(&video_buffer,
                           video_buffer_name,
                           video_buffer_size,
                           video_render_interval)) {
        goto end;
    }

    video_buffer_initialized = true;
    atexit(before_exit);
    decoder_init(&decoder, &video_buffer);
//    av_log_set_callback(av_log_callback);
    LOGI("start stream");
    stream_init(&stream, server.video_socket, &decoder);
    ret = stream_start(&stream);

end:
    // shutdown the sockets and kill the server
    server_disconnect(&server);

    // now that the sockets are shutdown, the stream and controller are
    // interrupted, we can join them
    if (video_buffer_initialized) {
        video_buffer_destroy(&video_buffer);
    }

    return ret;
}
