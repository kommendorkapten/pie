/* Automatically generated at Sat May 13 10:43:09 2017 */
/* Do not edit - things may break. */
#ifndef __12120_PIE_COLLECTION_H__
#define __12120_PIE_COLLECTION_H__
#include <sqlite3.h>
struct pie_collection
{
	int             col_id;
	char           *col_name;
	char           *col_path;
	int             hst_usr_id;
	int             hst_grp_id;
	int             hst_acl;
};
extern struct pie_collection *pie_collection_alloc(void);
extern void     pie_collection_free(struct pie_collection * this);
extern void     pie_collection_release(struct pie_collection * this);
extern int      pie_collection_create(sqlite3 * db, struct pie_collection * this);
extern int      pie_collection_read(sqlite3 * db, struct pie_collection * this);
extern int      pie_collection_update(sqlite3 * db, struct pie_collection * this);
extern int      pie_collection_delete(sqlite3 * db, struct pie_collection * this);

#endif				/* __12120_PIE_COLLECTION_H__ */
