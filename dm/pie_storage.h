/* Automatically generated at Sat Apr  8 17:25:59 2017 */
/* Do not edit - things may break. */
#ifndef __9332_PIE_STORAGE_H__
#define __9332_PIE_STORAGE_H__
#include <sqlite3.h>

#define PIE_STG_ONLINE   1
#define PIE_STG_NEARLINE 2
#define PIE_STG_THUMB    3
#define PIE_STG_PROXY    4
#define PIE_STG_EXPORT   5

struct pie_storage
{
	int             stg_id;
	char           *stg_name;
	int             stg_type;
	int             stg_hst_id;
};
extern struct pie_storage *pie_storage_alloc(void);
extern void     pie_storage_free(struct pie_storage * this);
extern void     pie_storage_release(struct pie_storage * this);
extern int      pie_storage_create(sqlite3 * db, struct pie_storage * this);
extern int      pie_storage_read(sqlite3 * db, struct pie_storage * this);
extern int      pie_storage_update(sqlite3 * db, struct pie_storage * this);
extern int      pie_storage_delete(sqlite3 * db, struct pie_storage * this);

#endif				/* __9332_PIE_STORAGE_H__ */
