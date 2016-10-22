#ifndef __CHAN_LOCK_H__
#define __CHAN_LOCK_H__

struct lock_s;
typedef struct lock_s* lock;

lock lk_create(void);

/**
 * @return 0 if the lock was taken. Non-zero otherwise.
 */
int lk_lock(lock);
void lk_unlock(lock);
void lk_destroy(lock);

#endif /* __CHAN_LOCK_H__ */

