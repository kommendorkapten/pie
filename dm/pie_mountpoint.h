/* Automatically generated at Sun Apr  2 19:14:02 2017 */
/* Do not edit - things may break. */
#ifndef __6223_PIE_MOUNTPOINT_H__
#define __6223_PIE_MOUNTPOINT_H__
#include <sqlite3.h>
struct pie_mountpoint
{
	int             mnt_hst_id;
	int             mnt_stg_id;
	char           *mnt_path;
};
extern struct pie_mountpoint *pie_mountpoint_alloc(void);
extern void     pie_mountpoint_free(struct pie_mountpoint * this);
extern void     pie_mountpoint_release(struct pie_mountpoint * this);
extern int      pie_mountpoint_create(sqlite3 * db,
                                      struct pie_mountpoint * this);
extern int      pie_mountpoint_read(sqlite3 * db,
                                    struct pie_mountpoint * this);
extern int      pie_mountpoint_find_host(sqlite3 * db,
                                         struct pie_mountpoint ** this,
                                         int hst_id,
                                         size_t len);
extern int      pie_mountpoint_update(sqlite3 * db,
                                      struct pie_mountpoint * this);
extern int      pie_mountpoint_delete(sqlite3 * db,
                                      struct pie_mountpoint * this);

#endif				/* __6223_PIE_MOUNTPOINT_H__ */
