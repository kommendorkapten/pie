/* Automatically generated at Tue Jun  6 08:24:33 2017 */
/* Do not edit - things may break. */
#ifndef __28052_PIE_COLLECTION_H__
#define __28052_PIE_COLLECTION_H__
#include <sqlite3.h>
#include "../pie_id.h"

struct llist;
struct pie_mob;

#ifdef PIE_PATH_LEN
# define COL_PATH_LEN PIE_PATH_LEN
#else
# define COL_PATH_LEN 256
#endif

struct pie_collection_asset
{
        struct pie_mob* mob;
        char developed;
};

struct pie_collection
{
	long            col_id;
	char            col_path[COL_PATH_LEN];
	int             col_usr_id;
	int             col_grp_id;
	int             col_acl;
        /* col_count does not exist in database and is not updated by any
           find/read calls. */
        int             col_count;
};
extern struct pie_collection *pie_collection_alloc(void);
extern void     pie_collection_free(struct pie_collection * this);
extern void     pie_collection_release(struct pie_collection * this);
extern int      pie_collection_create(sqlite3 * db, struct pie_collection * this);
extern int      pie_collection_read(sqlite3 * db, struct pie_collection * this);
/**
 * Get the number of members in a collection.
 * @param db handle.
 * @param collection to set count on.
 * @return >0 if no collection is found.
 *          0 on ok.
 *         <0 on error.
 */
extern int      pie_collection_read_count(sqlite3* db, struct pie_collection* this);

/**
 * Find a collection matching a specific path. The collection's path must
 * exactly match the provided string.
 * @param db.
 * @param null terminated string to search for.
 * @return a collection (if one found) or NULL.
 */
extern struct pie_collection* pie_collection_find_path(sqlite3 * db, const char * path);

/**
 * Return a list of all collections in the database.
 * @param db handle.
 * @return a list with pointers to all collections.
 */
extern struct llist* pie_collection_find_all(sqlite3 * db);

/**
 * Find all assets associated with a specific collection.
 * @param datbase.
 * @param collection id.
 * @return A list of struct pie_collection_assets. The list may be empty.
 *         NULL if an error occured.
 *
 */
extern struct llist* pie_collection_find_assets(sqlite3* db, pie_id coll);
extern int      pie_collection_update(sqlite3 * db, struct pie_collection * this);
extern int      pie_collection_delete(sqlite3 * db, struct pie_collection * this);

extern struct pie_collection_asset* pie_collection_asset_alloc(void);
extern void pie_collection_asset_release(struct pie_collection_asset*);
extern void pie_collection_asset_free(struct pie_collection_asset*);

#endif				/* __28052_PIE_COLLECTION_H__ */
