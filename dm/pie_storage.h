/* Automatically generated at Sat Apr  8 17:25:59 2017 */
/* Do not edit - things may break. */
#ifndef __9332_PIE_STORAGE_H__
#define __9332_PIE_STORAGE_H__
#include <sqlite3.h>

enum pie_storage_type{
        PIE_STG_INVALID,
        PIE_STG_ONLINE,
        PIE_STG_NEARLINE,
        PIE_STG_THUMB,
        PIE_STG_PROXY,
        PIE_STG_EXPORT,
        PIE_STG_MAX
};

struct pie_storage
{
	int                   stg_id;
	char                  *stg_name;
	enum pie_storage_type stg_type;
	int                   stg_hst_id;
};
extern struct pie_storage *pie_storage_alloc(void);
extern void     pie_storage_free(struct pie_storage * this);
extern void     pie_storage_release(struct pie_storage * this);
extern int      pie_storage_create(sqlite3 * db, struct pie_storage * this);
extern int      pie_storage_read(sqlite3 * db, struct pie_storage * this);
extern int      pie_storage_update(sqlite3 * db, struct pie_storage * this);
extern int      pie_storage_delete(sqlite3 * db, struct pie_storage * this);
extern const char* pie_storage_type(enum pie_storage_type);

#endif				/* __9332_PIE_STORAGE_H__ */
