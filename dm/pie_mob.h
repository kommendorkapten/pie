/* Automatically generated at Mon Jun  5 19:57:21 2017 */
/* Do not edit - things may break. */
#ifndef __27782_PIE_MOB_H__
#define __27782_PIE_MOB_H__
#include <sqlite3.h>

enum pie_mob_rating
{
        PIE_MOB_RATING_0,
        PIE_MOB_RATING_1,
        PIE_MOB_RATING_2,
        PIE_MOB_RATING_3,
        PIE_MOB_RATING_4,
        PIE_MOB_RATING_5,
        PIE_MOB_RATING_COUNT
};

enum pie_mob_color
{
        PIE_MOB_COLOR_NONE,
        PIE_MOB_COLOR_RED,
        PIE_MOB_COLOR_GREEN,
        PIE_MOB_COLOR_BLUE,
        PIE_MOB_COLOR_YELLOW,
        PIE_MOB_COLOR_BLACK,
        PIE_MOB_COLOR_COUNT
};

struct pie_mob
{
	long            mob_id;
	long            mob_parent_mob_id;
	char           *mob_name;
	long            mob_capture_ts_millis;
	long            mob_added_ts_millis;
	short           mob_format;
	char            mob_color;
	char            mob_rating;
};
extern struct pie_mob *pie_mob_alloc(void);
extern void     pie_mob_free(struct pie_mob * this);
extern void     pie_mob_release(struct pie_mob * this);
extern int      pie_mob_create(sqlite3 * db, struct pie_mob * this);
extern int      pie_mob_read(sqlite3 * db, struct pie_mob * this);
extern int      pie_mob_update(sqlite3 * db, struct pie_mob * this);
extern int      pie_mob_delete(sqlite3 * db, struct pie_mob * this);

#endif				/* __27782_PIE_MOB_H__ */
