/* Automatically generated at Thu May 25 16:40:17 2017 */
/* Do not edit - things may break. */
#ifndef __19550_PIE_EXIF_DATA_H__
#define __19550_PIE_EXIF_DATA_H__
#include <sqlite3.h>
struct pie_exif_data
{
	long            ped_mob_id;
	char           *ped_artist;
	char           *ped_copyright;
	char           *ped_software;
        /* YYYY:mm::dd HH:MM:SS */
	char           *ped_date_time;
	char           *ped_lens_model;
	char           *ped_make;
	char           *ped_model;
	char           *ped_exposure_time;
        /* hundreds of a second i.e [0, 99) */
	int             ped_sub_sec_time;
	int             ped_x_dim;
	int             ped_y_dim;
	int             ped_iso;
	int             ped_gamma;
	int             ped_white_point;
	short           ped_orientation;
	short           ped_focal_len;
	short           ped_fnumber;
	short           ped_exposure_bias;
	short           ped_white_balance;
	short           ped_exposure_prog;
	short           ped_metering_mode;
	short           ped_flash;
	short           ped_exposure_mode;
	short           ped_color_space;
};
extern struct pie_exif_data *pie_exif_data_alloc(void);
extern void     pie_exif_data_free(struct pie_exif_data * this);
extern void     pie_exif_data_release(struct pie_exif_data * this);
extern int      pie_exif_data_create(sqlite3 * db, struct pie_exif_data * this);
extern int      pie_exif_data_read(sqlite3 * db, struct pie_exif_data * this);
extern int      pie_exif_data_update(sqlite3 * db, struct pie_exif_data * this);
extern int      pie_exif_data_delete(sqlite3 * db, struct pie_exif_data * this);

#endif				/* __19550_PIE_EXIF_DATA_H__ */
