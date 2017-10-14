/* Automatically generated at Sat Oct 14 19:42:38 2017 */
/* Do not edit - things may break. */
#ifndef __2022_PIE_DEV_PARAMS_H__
#define __2022_PIE_DEV_PARAMS_H__
#include <sqlite3.h>

/* Make sure in sync with PIE_MQ_MAX_UPD in pie_mq_msg.h */
#define PIE_DEV_PARAMS_LEN 1024

struct pie_dev_params
{
	long pdp_mob_id;
	char pdp_settings[PIE_DEV_PARAMS_LEN];
};
extern struct pie_dev_params *pie_dev_params_alloc(void);
extern void     pie_dev_params_free(struct pie_dev_params * this);
extern void     pie_dev_params_release(struct pie_dev_params * this);
extern int      pie_dev_params_create(sqlite3 * db, struct pie_dev_params * this);
extern int      pie_dev_params_read(sqlite3 * db, struct pie_dev_params * this);
extern int      pie_dev_params_update(sqlite3 * db, struct pie_dev_params * this);
extern int      pie_dev_params_delete(sqlite3 * db, struct pie_dev_params * this);

/**
 * Test if the given id exists in the database.
 * @param database.
 * @param pie_id to test for existance.
 * @return  <0 on error.
 *          0 if non existing.
 *          1 if id exists.
 */
extern int pie_dev_params_exist(sqlite3* db, long id);

#endif				/* __2022_PIE_DEV_PARAMS_H__ */
