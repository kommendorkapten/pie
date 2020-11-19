#ifndef __CHAN_LOCK_H__
#define __CHAN_LOCK_H__

struct lock;

struct lock* lk_create(void);

/**
 * @return 0 if the lock was taken. Non-zero otherwise.
 */
int lk_lock(struct lock*);
void lk_unlock(struct lock*);
void lk_destroy(struct lock*);

#endif /* __CHAN_LOCK_H__ */

