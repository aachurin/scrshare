#include "scrshare.h"

#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <libavformat/avformat.h>

#include "compat.h"
#include "log.h"

#define DEFAULT_VIDEO_SERVER_PORT 27183
#define DEFAULT_RECEIVER_SERVER_PORT 27184


struct args {
    bool help;
    uint16_t video_server_port;
    uint16_t receiver_server_port;
    uint16_t video_render_interval;
    int log_level;
};

static void usage(const char *arg0) {
    fprintf(stderr,
        "Usage: %s [options]\n"
        "\n"
        "Options:\n"
        "\n"
        "    -h, --help\n"
        "        Print this help.\n"
        "\n"
        "    -v, --video_server_port port\n"
        "        Video server TCP port.\n"
        "        Default is %d.\n"
        "\n"
        "    -r, --receiver_server_port port\n"
        "        Receiver server TCP port.\n"
        "        Default is %d.\n"
        "\n"
        "    -i, --video-render-interval N\n"
        "        Render interval to the video buffer (0 for disabling)\n"
        "\n"
        "    -l, --loglevel [DEBUG|INFO|ERROR|CRITICAL]\n"
        "        Logging level (default: 'INFO')\n"
        "\n",
        arg0,
        DEFAULT_VIDEO_SERVER_PORT,
        DEFAULT_RECEIVER_SERVER_PORT);
}

static bool
parse_port(char *optarg, uint16_t *port) {
    char *endptr;
    if (*optarg == '\0') {
        LOGE("Invalid port, parameter is empty");
        return false;
    }
    long value = strtol(optarg, &endptr, 0);
    if (*endptr != '\0') {
        LOGE("Invalid port: %s", optarg);
        return false;
    }
    if (value & ~0xffff) {
        LOGE("Port out of range: %ld", value);
        return false;
    }

    *port = (uint16_t) value;
    return true;
}

static bool
parse_video_render_interval(char *optarg, uint16_t *num) {
    char *endptr;
    if (*optarg == '\0') {
        LOGE("Invalid frame number, parameter is empty");
        return false;
    }
    long value = strtol(optarg, &endptr, 0);
    if (*endptr != '\0') {
        LOGE("Invalid frame number: %s", optarg);
        return false;
    }
    if (value & ~0xffff) {
        LOGE("Frame number out of range: %ld", value);
        return false;
    }

    *num = (uint16_t) value;
    return true;
}

static bool
parse_log_level(char* optarg, int *log_level) {
    if (*optarg == '\0') {
        LOGE("Invalid frame number parameter is empty");
        return false;
    }
    if (strcmp(optarg, "DEBUG") == 0) {
        *log_level = LEVEL_DEBUG;
    } else if (strcmp(optarg, "INFO") == 0) {
        *log_level = LEVEL_INFO;
    } else if (strcmp(optarg, "ERROR") == 0) {
        *log_level = LEVEL_ERROR;
    } else if (strcmp(optarg, "CRITICAL") == 0) {
        *log_level = LEVEL_CRITICAL;
    } else {
        LOGE("Invalid log level: %s", optarg);
        return false;
    }
    return true;
}

static bool
parse_args(struct args *args, int argc, char *argv[]) {
    static const struct option long_options[] = {
        {"help",                  no_argument,       NULL, 'h'},
        {"video-server-port",     required_argument, NULL, 'v'},
        {"receiver-server-port",  required_argument, NULL, 'r'},
        {"video-render-interval", required_argument, NULL, 'i'},
        {"log-level",             required_argument, NULL, 'l'},
        {NULL,                    0,                 NULL, 0  },
    };
    int c;
    while ((c = getopt_long(argc, argv, "hv:r:i:l:", long_options, NULL)) != -1) {
        switch (c) {
            case 'h':
                args->help = true;
                break;
            case 'v':
                if (!parse_port(optarg, &args->video_server_port)) {
                    return false;
                }
                break;
            case 'r':
                if (!parse_port(optarg, &args->receiver_server_port)) {
                    return false;
                }
                break;
            case 'i':
                if (!parse_video_render_interval(optarg, &args->video_render_interval)) {
                    return false;
                }
                break;
            case 'l':
                if (!parse_log_level(optarg, &args->log_level)) {
                    return false;
                }
                break;
            default:
                // getopt prints the error message on stderr
                return false;
        }
    }

    int index = optind;
    if (index < argc) {
        LOGE("Unexpected additional argument: %s", argv[index]);
        return false;
    }

    return true;
}

int
main(int argc, char *argv[]) {
    set_log_level(LEVEL_INFO);

#ifdef _WIN64
    // disable buffering, we want logs immediately
    // even line buffering (setvbuf() with mode _IOLBF) is not sufficient
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
#endif
    struct args args = {
        .help = false,
        .video_server_port = DEFAULT_VIDEO_SERVER_PORT,
        .receiver_server_port = DEFAULT_RECEIVER_SERVER_PORT,
        .video_render_interval = 0
    };

    if (!parse_args(&args, argc, argv)) {
        return 1;
    }

    if (args.help) {
        usage(argv[0]);
        return 0;
    }

    set_log_level(args.log_level);

    LOGI("scrshare 1.0 started ...");

#ifdef SCRSHARE_LAVF_REQUIRES_REGISTER_ALL
    av_register_all();
#endif

    if (avformat_network_init()) {
        return 1;
    }

    int res = scrshare(
        args.video_server_port,
        args.receiver_server_port,
        args.video_render_interval
        ) ? 0 : 1;

    avformat_network_deinit();

    return res;
}
