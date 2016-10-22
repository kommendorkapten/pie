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
        lock l;
#endif
};

struct chan_msg;

/**
 * Read one msg from fd.
 * @return
 *     0: Message successfully read.
 *     -1: Error occured.
 *     EBADF: Channel is closed.
 *     EAGAIN: Nothing to read.
 */
int read_msg(struct chan*, struct chan_msg*) ;

#endif /* __CHAN_DEF_H__ */
