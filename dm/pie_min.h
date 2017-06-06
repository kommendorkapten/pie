/* Automatically generated at Tue Jun  6 08:53:08 2017 */
/* Do not edit - things may break. */
#ifndef __28230_PIE_MIN_H__
#define __28230_PIE_MIN_H__
#include <sqlite3.h>
struct pie_min
{
	long            min_id;
	long            min_mob_id;
	long            min_added_ts_millis;
	int             min_stg_id;
	char           *min_path;
};
extern struct pie_min *pie_min_alloc(void);
extern void     pie_min_free(struct pie_min * this);
extern void     pie_min_release(struct pie_min * this);
extern int      pie_min_create(sqlite3 * db, struct pie_min * this);
extern int      pie_min_read(sqlite3 * db, struct pie_min * this);
extern int      pie_min_update(sqlite3 * db, struct pie_min * this);
extern int      pie_min_delete(sqlite3 * db, struct pie_min * this);

#endif				/* __28230_PIE_MIN_H__ */
