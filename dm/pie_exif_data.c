/* Automatically generated at Thu May 25 16:40:17 2017 */
/* Do not edit - things may break. */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "pie_exif_data.h"
struct pie_exif_data *
pie_exif_data_alloc(void)
{
	struct pie_exif_data *this = malloc(sizeof(struct pie_exif_data));

	this->ped_artist = NULL;
	this->ped_copyright = NULL;
	this->ped_software = NULL;
	this->ped_date_time = NULL;
	this->ped_lens_model = NULL;
	this->ped_make = NULL;
	this->ped_model = NULL;
	this->ped_exposure_time = NULL;
	return this;
}
void 
pie_exif_data_free(struct pie_exif_data * this)
{
	assert(this);
	pie_exif_data_release(this);
	free(this);
}
void 
pie_exif_data_release(struct pie_exif_data * this)
{
	assert(this);
	if (this->ped_artist)
	{
		free(this->ped_artist);
		this->ped_artist = NULL;
	}
	if (this->ped_copyright)
	{
		free(this->ped_copyright);
		this->ped_copyright = NULL;
	}
	if (this->ped_software)
	{
		free(this->ped_software);
		this->ped_software = NULL;
	}
	if (this->ped_date_time)
	{
		free(this->ped_date_time);
		this->ped_date_time = NULL;
	}
	if (this->ped_lens_model)
	{
		free(this->ped_lens_model);
		this->ped_lens_model = NULL;
	}
	if (this->ped_make)
	{
		free(this->ped_make);
		this->ped_make = NULL;
	}
	if (this->ped_model)
	{
		free(this->ped_model);
		this->ped_model = NULL;
	}
	if (this->ped_exposure_time)
	{
		free(this->ped_exposure_time);
		this->ped_exposure_time = NULL;
	}
}
int 
pie_exif_data_create(sqlite3 * db, struct pie_exif_data * this)
{
	char           *q = "INSERT INTO pie_exif_data (ped_mob_id,ped_artist,ped_copyright,ped_software,ped_date_time,ped_lens_model,ped_make,ped_model,ped_exposure_time,ped_sub_sec_time,ped_x_dim,ped_y_dim,ped_iso,ped_gamma,ped_white_point,ped_orientation,ped_focal_len,ped_fnumber,ped_exposure_bias,ped_white_balance,ped_exposure_prog,ped_metering_mode,ped_flash,ped_exposure_mode,ped_color_space) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
	sqlite3_stmt   *pstmt;
	int             ret;
	int             retf;

	assert(this);
	/* Check if a key is expected to be generated or not */
	if (this->ped_mob_id == 0)
	{
		q = "INSERT INTO pie_exif_data (ped_artist,ped_copyright,ped_software,ped_date_time,ped_lens_model,ped_make,ped_model,ped_exposure_time,ped_sub_sec_time,ped_x_dim,ped_y_dim,ped_iso,ped_gamma,ped_white_point,ped_orientation,ped_focal_len,ped_fnumber,ped_exposure_bias,ped_white_balance,ped_exposure_prog,ped_metering_mode,ped_flash,ped_exposure_mode,ped_color_space) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
	}
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	if (this->ped_mob_id == 0)
	{
		ret = sqlite3_bind_text(pstmt, 1, this->ped_artist, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 2, this->ped_copyright, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 3, this->ped_software, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 4, this->ped_date_time, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 5, this->ped_lens_model, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 6, this->ped_make, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 7, this->ped_model, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 8, this->ped_exposure_time, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 9, (int) this->ped_sub_sec_time);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 10, (int) this->ped_x_dim);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 11, (int) this->ped_y_dim);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 12, (int) this->ped_iso);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 13, (int) this->ped_gamma);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 14, (int) this->ped_white_point);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 15, (int) this->ped_orientation);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 16, (int) this->ped_focal_len);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 17, (int) this->ped_fnumber);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 18, (int) this->ped_exposure_bias);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 19, (int) this->ped_white_balance);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 20, (int) this->ped_exposure_prog);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 21, (int) this->ped_metering_mode);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 22, (int) this->ped_flash);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 23, (int) this->ped_exposure_mode);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 24, (int) this->ped_color_space);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
	}
	else
	{
		ret = sqlite3_bind_int64(pstmt, 1, this->ped_mob_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 2, this->ped_artist, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 3, this->ped_copyright, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 4, this->ped_software, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 5, this->ped_date_time, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 6, this->ped_lens_model, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 7, this->ped_make, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 8, this->ped_model, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 9, this->ped_exposure_time, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 10, (int) this->ped_sub_sec_time);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 11, (int) this->ped_x_dim);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 12, (int) this->ped_y_dim);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 13, (int) this->ped_iso);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 14, (int) this->ped_gamma);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 15, (int) this->ped_white_point);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 16, (int) this->ped_orientation);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 17, (int) this->ped_focal_len);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 18, (int) this->ped_fnumber);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 19, (int) this->ped_exposure_bias);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 20, (int) this->ped_white_balance);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 21, (int) this->ped_exposure_prog);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 22, (int) this->ped_metering_mode);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 23, (int) this->ped_flash);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 24, (int) this->ped_exposure_mode);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 25, (int) this->ped_color_space);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
	}
	ret = sqlite3_step(pstmt);
	if (ret != SQLITE_DONE)
	{
                printf("%s\n", sqlite3_errmsg(db));
		ret = -1;
		goto cleanup;
	}
	if (this->ped_mob_id == 0)
	{
		/* Extract last generated key. */
		/* I repeat, this is *NOT* thread safe. */
		this->ped_mob_id = (long) sqlite3_last_insert_rowid(db);
	}
	ret = 0;
cleanup:
	retf = sqlite3_finalize(pstmt);
	if (retf != SQLITE_OK)
	{
		ret = -1;
	}
	return ret;
}
int 
pie_exif_data_read(sqlite3 * db, struct pie_exif_data * this)
{
	char           *q = "SELECT ped_artist,ped_copyright,ped_software,ped_date_time,ped_lens_model,ped_make,ped_model,ped_exposure_time,ped_sub_sec_time,ped_x_dim,ped_y_dim,ped_iso,ped_gamma,ped_white_point,ped_orientation,ped_focal_len,ped_fnumber,ped_exposure_bias,ped_white_balance,ped_exposure_prog,ped_metering_mode,ped_flash,ped_exposure_mode,ped_color_space FROM pie_exif_data WHERE ped_mob_id = ?";
	sqlite3_stmt   *pstmt;
	int             ret;
	int             retf;
	const unsigned char *c;
	int             br;

	assert(this);
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int64(pstmt, 1, this->ped_mob_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_step(pstmt);
	if (ret == SQLITE_DONE)
	{
		ret = 1;
		goto cleanup;
	}
	if (ret != SQLITE_ROW)
	{
		ret = -1;
		goto cleanup;
	}
	/* Force reading text into memory, and ge the length */
	/* of the string (null terminator not included). */
	/* Allocate memory and copy string to destination, */
	/* and set the null terminator., */
	c = sqlite3_column_text(pstmt, 0);
	br = sqlite3_column_bytes(pstmt, 0);
	this->ped_artist = malloc(br + 1);
	memcpy(this->ped_artist, c, br);
	this->ped_artist[br] = '\0';
	/* Force reading text into memory, and ge the length */
	/* of the string (null terminator not included). */
	/* Allocate memory and copy string to destination, */
	/* and set the null terminator., */
	c = sqlite3_column_text(pstmt, 1);
	br = sqlite3_column_bytes(pstmt, 1);
	this->ped_copyright = malloc(br + 1);
	memcpy(this->ped_copyright, c, br);
	this->ped_copyright[br] = '\0';
	/* Force reading text into memory, and ge the length */
	/* of the string (null terminator not included). */
	/* Allocate memory and copy string to destination, */
	/* and set the null terminator., */
	c = sqlite3_column_text(pstmt, 2);
	br = sqlite3_column_bytes(pstmt, 2);
	this->ped_software = malloc(br + 1);
	memcpy(this->ped_software, c, br);
	this->ped_software[br] = '\0';
	/* Force reading text into memory, and ge the length */
	/* of the string (null terminator not included). */
	/* Allocate memory and copy string to destination, */
	/* and set the null terminator., */
	c = sqlite3_column_text(pstmt, 3);
	br = sqlite3_column_bytes(pstmt, 3);
	this->ped_date_time = malloc(br + 1);
	memcpy(this->ped_date_time, c, br);
	this->ped_date_time[br] = '\0';
	/* Force reading text into memory, and ge the length */
	/* of the string (null terminator not included). */
	/* Allocate memory and copy string to destination, */
	/* and set the null terminator., */
	c = sqlite3_column_text(pstmt, 4);
	br = sqlite3_column_bytes(pstmt, 4);
	this->ped_lens_model = malloc(br + 1);
	memcpy(this->ped_lens_model, c, br);
	this->ped_lens_model[br] = '\0';
	/* Force reading text into memory, and ge the length */
	/* of the string (null terminator not included). */
	/* Allocate memory and copy string to destination, */
	/* and set the null terminator., */
	c = sqlite3_column_text(pstmt, 5);
	br = sqlite3_column_bytes(pstmt, 5);
	this->ped_make = malloc(br + 1);
	memcpy(this->ped_make, c, br);
	this->ped_make[br] = '\0';
	/* Force reading text into memory, and ge the length */
	/* of the string (null terminator not included). */
	/* Allocate memory and copy string to destination, */
	/* and set the null terminator., */
	c = sqlite3_column_text(pstmt, 6);
	br = sqlite3_column_bytes(pstmt, 6);
	this->ped_model = malloc(br + 1);
	memcpy(this->ped_model, c, br);
	this->ped_model[br] = '\0';
	/* Force reading text into memory, and ge the length */
	/* of the string (null terminator not included). */
	/* Allocate memory and copy string to destination, */
	/* and set the null terminator., */
	c = sqlite3_column_text(pstmt, 7);
	br = sqlite3_column_bytes(pstmt, 7);
	this->ped_exposure_time = malloc(br + 1);
	memcpy(this->ped_exposure_time, c, br);
	this->ped_exposure_time[br] = '\0';
	this->ped_sub_sec_time = (int) sqlite3_column_int(pstmt, 8);
	this->ped_x_dim = (int) sqlite3_column_int(pstmt, 9);
	this->ped_y_dim = (int) sqlite3_column_int(pstmt, 10);
	this->ped_iso = (int) sqlite3_column_int(pstmt, 11);
	this->ped_gamma = (int) sqlite3_column_int(pstmt, 12);
	this->ped_white_point = (int) sqlite3_column_int(pstmt, 13);
	this->ped_orientation = (short) sqlite3_column_int(pstmt, 14);
	this->ped_focal_len = (short) sqlite3_column_int(pstmt, 15);
	this->ped_fnumber = (short) sqlite3_column_int(pstmt, 16);
	this->ped_exposure_bias = (short) sqlite3_column_int(pstmt, 17);
	this->ped_white_balance = (short) sqlite3_column_int(pstmt, 18);
	this->ped_exposure_prog = (short) sqlite3_column_int(pstmt, 19);
	this->ped_metering_mode = (short) sqlite3_column_int(pstmt, 20);
	this->ped_flash = (short) sqlite3_column_int(pstmt, 21);
	this->ped_exposure_mode = (short) sqlite3_column_int(pstmt, 22);
	this->ped_color_space = (short) sqlite3_column_int(pstmt, 23);
	ret = 0;
cleanup:
	retf = sqlite3_finalize(pstmt);
	if (retf != SQLITE_OK)
	{
		ret = -1;
	}
	return ret;
}
int 
pie_exif_data_update(sqlite3 * db, struct pie_exif_data * this)
{
	char           *q = "UPDATE pie_exif_data SET ped_artist = ?,ped_copyright = ?,ped_software = ?,ped_date_time = ?,ped_lens_model = ?,ped_make = ?,ped_model = ?,ped_exposure_time = ?,ped_sub_sec_time = ?,ped_x_dim = ?,ped_y_dim = ?,ped_iso = ?,ped_gamma = ?,ped_white_point = ?,ped_orientation = ?,ped_focal_len = ?,ped_fnumber = ?,ped_exposure_bias = ?,ped_white_balance = ?,ped_exposure_prog = ?,ped_metering_mode = ?,ped_flash = ?,ped_exposure_mode = ?,ped_color_space = ? WHERE ped_mob_id = ?";
	sqlite3_stmt   *pstmt;
	int             ret;
	int             retf;

	assert(this);
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 1, this->ped_artist, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 2, this->ped_copyright, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 3, this->ped_software, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 4, this->ped_date_time, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 5, this->ped_lens_model, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 6, this->ped_make, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 7, this->ped_model, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 8, this->ped_exposure_time, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 9, (int) this->ped_sub_sec_time);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 10, (int) this->ped_x_dim);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 11, (int) this->ped_y_dim);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 12, (int) this->ped_iso);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 13, (int) this->ped_gamma);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 14, (int) this->ped_white_point);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 15, (int) this->ped_orientation);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 16, (int) this->ped_focal_len);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 17, (int) this->ped_fnumber);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 18, (int) this->ped_exposure_bias);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 19, (int) this->ped_white_balance);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 20, (int) this->ped_exposure_prog);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 21, (int) this->ped_metering_mode);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 22, (int) this->ped_flash);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 23, (int) this->ped_exposure_mode);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 24, (int) this->ped_color_space);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int64(pstmt, 25, this->ped_mob_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_step(pstmt);
	if (ret != SQLITE_DONE)
	{
		ret = -1;
		goto cleanup;
	}
	ret = 0;
cleanup:
	retf = sqlite3_finalize(pstmt);
	if (retf != SQLITE_OK)
	{
		ret = -1;
	}
	return ret;
}
int 
pie_exif_data_delete(sqlite3 * db, struct pie_exif_data * this)
{
	char           *q = "DELETE FROM pie_exif_data WHERE ped_mob_id = ?";
	sqlite3_stmt   *pstmt;
	int             ret;
	int             retf;

	assert(this);
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int64(pstmt, 1, this->ped_mob_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_step(pstmt);
	if (ret != SQLITE_DONE)
	{
		ret = -1;
		goto cleanup;
	}
	ret = 0;
cleanup:
	retf = sqlite3_finalize(pstmt);
	if (retf != SQLITE_OK)
	{
		ret = -1;
	}
	return ret;
}
