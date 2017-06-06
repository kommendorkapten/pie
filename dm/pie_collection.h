/* Automatically generated at Tue Jun  6 08:24:33 2017 */
/* Do not edit - things may break. */
#ifndef __28052_PIE_COLLECTION_H__
#define __28052_PIE_COLLECTION_H__
#include <sqlite3.h>

struct llist;

struct pie_collection
{
	long            col_id;
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
extern struct pie_collection* pie_collection_find_path(sqlite3 * db, const char
extern struct llist* pie_collection_find_all(sqlite3 * db);
extern int      pie_collection_update(sqlite3 * db, struct pie_collection * this);
extern int      pie_collection_delete(sqlite3 * db, struct pie_collection * this);

#endif				/* __28052_PIE_COLLECTION_H__ */
