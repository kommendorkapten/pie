#ifndef __CHAN_DEF_H__
#define __CHAN_DEF_H__

#include <pthread.h>
#ifdef MT_SAFE
# include "lock.h"
#endif

#define READ_FD 0
#define WRITE_FD 1

struct chan
{
        pthread_t* fan_in;
        int fds[2];
#ifdef MT_SAFE
        struct lock* l;
#endif
};

struct chan_msg;

#endif /* __CHAN_DEF_H__ */
