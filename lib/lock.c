#include "lock.h"
#include <stdlib.h>
#ifdef LOCK_FREE
# include <atomic.h>
# include <stdint.h>
#else
# include <pthread.h>
#endif

struct lock
{
#ifdef LOCK_FREE
        uint32_t l;
#else
        pthread_mutex_t l;
#endif
};

struct lock* lk_create(void)
{
        struct lock* l = malloc(sizeof(struct lock));

#ifdef LOCK_FREE
        l->l = 0;
#else
        pthread_mutex_init(&l->l, NULL);
#endif

        return l;
}

int lk_lock(struct lock* l)
{
#ifdef LOCK_FREE
        for (;;)
        {
                int r = atomic_cas_32(&l->l, 0, 1);

                if (r == 0)
                {
                        break;
                }
        }
        return 0;
#else
        return pthread_mutex_lock(&l->l);
#endif
}

void lk_unlock(struct lock* l)
{
#ifdef LOCK_FREE
        l->l = 0;
#else
	pthread_mutex_unlock(&l->l);
#endif
}

void lk_destroy(struct lock* l)
{
#ifdef LOCK_FREE
#else
        pthread_mutex_destroy(&l->l);
#endif
        free(l);
}
