/* Automatically generated at Sun May 14 14:46:11 2017 */
/* Do not edit - things may break. */
#ifndef __13160_PIE_COLLECTION_MEMBER_H__
#define __13160_PIE_COLLECTION_MEMBER_H__
#include <sqlite3.h>
struct pie_collection_member
{
	int             cmb_col_id;
	long            cmb_mob_id;
};
extern struct pie_collection_member *pie_collection_member_alloc(void);
extern void     pie_collection_member_free(struct pie_collection_member * this);
extern void     pie_collection_member_release(struct pie_collection_member * this);
extern int      pie_collection_member_create(sqlite3 * db, struct pie_collection_member * this);
extern int      pie_collection_member_read(sqlite3 * db, struct pie_collection_member * this);
extern int      pie_collection_member_update(sqlite3 * db, struct pie_collection_member * this);
extern int      pie_collection_member_delete(sqlite3 * db, struct pie_collection_member * this);

#endif				/* __13160_PIE_COLLECTION_MEMBER_H__ */
