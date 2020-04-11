#ifndef STREAM_H
#define STREAM_H

#include <stdbool.h>
#include <stdint.h>
#include <libavformat/avformat.h>

#include "net.h"

struct video_buffer;

struct stream {
    socket_t socket;
    struct video_buffer *video_buffer;
    struct decoder *decoder;
    struct recorder *recorder;
    AVCodecContext *codec_ctx;
    AVCodecParserContext *parser;
    bool has_pending;
    AVPacket pending;
};

void
stream_init(struct stream *stream, socket_t socket, struct decoder *decoder);

bool
stream_start(struct stream *stream);

#endif
