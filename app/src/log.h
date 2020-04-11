#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define LEVEL_DEBUG 0
#define LEVEL_INFO 1
#define LEVEL_WARN 2
#define LEVEL_ERROR 3
#define LEVEL_CRITICAL 4

#define LOGD(...) if (get_log_level() <= LEVEL_DEBUG) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }
#define LOGI(...) if (get_log_level() <= LEVEL_INFO) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }
#define LOGW(...) if (get_log_level() <= LEVEL_WARN) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }
#define LOGE(...) if (get_log_level() <= LEVEL_ERROR) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }
#define LOGC(...) if (get_log_level() <= LEVEL_CRITICAL) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }

void set_log_level(int level);
int get_log_level();

#endif
