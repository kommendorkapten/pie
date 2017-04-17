/* Automatically generated at Sun Apr  2 19:14:02 2017 */
/* Do not edit - things may break. */
#ifndef __6223_PIE_HOST_H__
#define __6223_PIE_HOST_H__
#include <sqlite3.h>
struct pie_host
{
	int             hst_id;
	char           *hst_name;
	char           *hst_fqdn;
};
extern struct pie_host *pie_host_alloc(void);
extern void     pie_host_free(struct pie_host * this);
extern void     pie_host_release(struct pie_host * this);
extern int      pie_host_create(sqlite3 * db, struct pie_host * this);
extern int      pie_host_read(sqlite3 * db, struct pie_host * this);
extern int      pie_host_find_name(sqlite3 * db, struct pie_host * this);
extern int      pie_host_update(sqlite3 * db, struct pie_host * this);
extern int      pie_host_delete(sqlite3 * db, struct pie_host * this);

#endif				/* __6223_PIE_HOST_H__ */
