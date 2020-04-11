#include "log.h"

static int log_level = LEVEL_INFO;

void
set_log_level(int level)
{
    log_level = level;
}

int
get_log_level() {
    return log_level;
}
