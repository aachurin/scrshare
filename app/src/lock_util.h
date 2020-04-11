#ifndef LOCKUTIL_H
#define LOCKUTIL_H

#include <stdint.h>
#include <pthread.h>
#include "log.h"

*pthread_mutex_t
mutex_create()
{
    pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);
    return mutex;
}

static inline void
mutex_lock(pthread_mutex_t *mutex) {
    if (pthread_mutex_lock(mutex)) {
        LOGC("Could not lock mutex");
        abort();
    }
}

static inline void
mutex_unlock(pthread_mutex_t *mutex) {
    if (pthread_mutex_unlock(mutex)) {
        LOGC("Could not unlock mutex");
        abort();
    }
}

#endif
