#include "decoder.h"

#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <unistd.h>

#include "compat.h"
#include "buffer_util.h"
#include "video_buffer.h"
#include "log.h"

void
decoder_init(struct decoder *decoder, struct video_buffer *vb) {
    decoder->video_buffer = vb;
}

bool
decoder_open(struct decoder *decoder, const AVCodec *codec) {
    decoder->codec_ctx = avcodec_alloc_context3(codec);
    if (!decoder->codec_ctx) {
        LOGC("Could not allocate decoder context");
        return false;
    }

    if (avcodec_open2(decoder->codec_ctx, codec, NULL) < 0) {
        LOGE("Could not open codec");
        avcodec_free_context(&decoder->codec_ctx);
        return false;
    }

    return true;
}

void
decoder_close(struct decoder *decoder) {
    avcodec_close(decoder->codec_ctx);
    avcodec_free_context(&decoder->codec_ctx);
}

bool
decoder_push(struct decoder *decoder, const AVPacket *packet) {
// the new decoding/encoding API has been introduced by:
// <http://git.videolan.org/?p=ffmpeg.git;a=commitdiff;h=7fc329e2dd6226dfecaa4a1d7adf353bf2773726>
#ifdef SCRSHARE_LAVF_HAS_NEW_ENCODING_DECODING_API
    int ret;
    if ((ret = avcodec_send_packet(decoder->codec_ctx, packet)) < 0) {
        LOGE("Could not send video packet: %d", ret);
        return false;
    }
    ret = avcodec_receive_frame(decoder->codec_ctx,
                                decoder->video_buffer->decoding_frame);
    if (!ret) {
        // a frame was received
        video_buffer_offer_decoded_frame(decoder->video_buffer);
    } else if (ret != AVERROR(EAGAIN)) {
        LOGE("Could not receive video frame: %d", ret);
        return false;
    }
#else
    int got_picture;
    int len = avcodec_decode_video2(decoder->codec_ctx,
                                    decoder->video_buffer->decoding_frame,
                                    &got_picture,
                                    packet);
    if (len < 0) {
        LOGE("Could not decode video packet: %d", len);
        return false;
    }
    if (got_picture) {
        video_buffer_offer_decoded_frame(decoder->video_buffer);
    }
#endif
    return true;
}
