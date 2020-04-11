#include "scrshare.h"

#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <libavformat/avformat.h>

#include "compat.h"
#include "log.h"

#define DEFAULT_LOCAL_PORT 27183

struct args {
    bool help;
    uint16_t port;
    const char *video_name;
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
        "    -p, --port port\n"
        "        Set the TCP port of server.\n"
        "        Default is %d.\n"
        "\n"
        "    -v, --video text\n"
        "        Set name for shared video buffer (default: '/android_video').\n"
        "\n"
        "    -i, --video-render-interval N\n"
        "        Render interval to the video buffer (0 for disabling)\n"
        "\n"
        "    -l, --loglevel [DEBUG|INFO|ERROR|CRITICAL]\n"
        "        Logging level (default: 'INFO')\n"
        "\n",
        arg0,
        DEFAULT_LOCAL_PORT);
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

const char DEFAULT_VIDEO_NAME[] = "/android_video";

static bool
parse_args(struct args *args, int argc, char *argv[]) {
    static const struct option long_options[] = {
        {"help",                  no_argument,       NULL, 'h'},
        {"port",                  required_argument, NULL, 'p'},
        {"video",                 required_argument, NULL, 'v'},
        {"video-render-interval", required_argument, NULL, 'i'},
        {"log-level",             required_argument, NULL, 'l'},
        {NULL,                    0,                 NULL, 0  },
    };
    int c;
    while ((c = getopt_long(argc, argv, "hp:v:i:l:", long_options, NULL)) != -1) {
        switch (c) {
            case 'h':
                args->help = true;
                break;
            case 'p':
                if (!parse_port(optarg, &args->port)) {
                    return false;
                }
                break;
            case 'v':
                args->video_name = optarg;
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

#ifdef __WINDOWS__
    // disable buffering, we want logs immediately
    // even line buffering (setvbuf() with mode _IOLBF) is not sufficient
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
#endif
    struct args args = {
        .help = false,
        .port = DEFAULT_LOCAL_PORT,
        .video_name = DEFAULT_VIDEO_NAME,
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

    int res = scrshare(args.port, args.video_name, args.video_render_interval) ? 0 : 1;
    avformat_network_deinit();

    return res;
}
