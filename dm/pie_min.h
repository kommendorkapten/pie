/* Automatically generated at Tue Jun  6 08:53:08 2017 */
/* Do not edit - things may break. */
#ifndef __28230_PIE_MIN_H__
#define __28230_PIE_MIN_H__
#include <sqlite3.h>
#include "../pie_id.h"

struct llist;

/* Keep SHA 1 hash in hex code + null terminator */
#define MIN_HASH_LEN (20 * 2 + 1)
#ifdef PIE_PATH_LEN
# define MIN_PATH_LEN PIE_PATH_LEN
#else
# define MIN_PATH_LEN 256
#endif

struct pie_min
{
	long            min_id;
	long            min_mob_id;
	long            min_added_ts_millis;
	int             min_stg_id;
        long            min_size;
	char            min_path[MIN_PATH_LEN];
        char            min_sha1_hash[MIN_HASH_LEN];
};

extern struct pie_min *pie_min_alloc(void);
extern void     pie_min_free(struct pie_min * this);
extern void     pie_min_release(struct pie_min * this);
extern int      pie_min_create(sqlite3 * db, struct pie_min * this);
extern int      pie_min_read(sqlite3 * db, struct pie_min * this);
extern int      pie_min_update(sqlite3 * db, struct pie_min * this);
extern int      pie_min_delete(sqlite3 * db, struct pie_min * this);

/**
 * Find all MINs associated with a specific MOB.
 * @param datbase.
 * @param MOB id.
 * @return a possible empty list with the matching MINs, or NULL if
 *         error occured.
 */
extern struct llist* pie_min_find_mob(sqlite3* db, pie_id mob_id);

#endif				/* __28230_PIE_MIN_H__ */
