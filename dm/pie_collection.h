/* Automatically generated at Sun May 14 09:07:18 2017 */
/* Do not edit - things may break. */
#ifndef __12915_PIE_COLLECTION_H__
#define __12915_PIE_COLLECTION_H__
#include <sqlite3.h>
struct pie_collection
{
	int             col_id;
	char           *col_path;
	int             col_usr_id;
	int             col_grp_id;
	int             col_acl;
};
extern struct pie_collection *pie_collection_alloc(void);
extern void     pie_collection_free(struct pie_collection * this);
extern void     pie_collection_release(struct pie_collection * this);
extern int      pie_collection_create(sqlite3 * db, struct pie_collection * this);
extern int      pie_collection_read(sqlite3 * db, struct pie_collection * this);
struct pie_collection* pie_collection_find_path(sqlite3 * db, const char * path);
extern int      pie_collection_update(sqlite3 * db, struct pie_collection * this);
extern int      pie_collection_delete(sqlite3 * db, struct pie_collection * this);

#endif				/* __12915_PIE_COLLECTION_H__ */
