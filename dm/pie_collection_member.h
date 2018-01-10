/* Automatically generated at Tue Jun  6 08:24:38 2017 */
/* Do not edit - things may break. */
#ifndef __28062_PIE_COLLECTION_MEMBER_H__
#define __28062_PIE_COLLECTION_MEMBER_H__
#include <sqlite3.h>
struct pie_collection_member
{
	long            cmb_col_id;
	long            cmb_mob_id;
};
extern struct pie_collection_member *pie_collection_member_alloc(void);
extern void     pie_collection_member_free(struct pie_collection_member * this);
extern void     pie_collection_member_release(struct pie_collection_member * this);
extern int      pie_collection_member_create(sqlite3 * db, struct pie_collection_member * this);
extern int      pie_collection_member_read(sqlite3 * db, struct pie_collection_member * this);
extern int      pie_collection_member_find_mob(sqlite3 * db, struct pie_collection_member* this);
extern int      pie_collection_member_update(sqlite3 * db, struct pie_collection_member * this);
extern int      pie_collection_member_delete(sqlite3 * db, struct pie_collection_member * this);
extern int      pie_collection_member_delete_mob(sqlite3 * db, long mob_id);

#endif				/* __28062_PIE_COLLECTION_MEMBER_H__ */
