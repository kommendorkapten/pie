/* Automatically generated at Mon Jun  5 19:57:21 2017 */
/* Do not edit - things may break. */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pie_mob.h"
struct pie_mob *
pie_mob_alloc(void)
{
	struct pie_mob *this = malloc(sizeof(struct pie_mob));

	this->mob_name = NULL;
	return this;
}
void 
pie_mob_free(struct pie_mob * this)
{
	assert(this);
	pie_mob_release(this);
	free(this);
}
void 
pie_mob_release(struct pie_mob * this)
{
	assert(this);
	if (this->mob_name)
	{
		free(this->mob_name);
		this->mob_name = NULL;
	}
}
int 
pie_mob_create(sqlite3 * db, struct pie_mob * this)
{
	char           *q = "INSERT INTO pie_mob (mob_id,mob_parent_mob_id,mob_name,mob_capture_ts_millis,mob_added_ts_millis,mob_format,mob_color,mob_rating) VALUES (?,?,?,?,?,?,?,?)";
	sqlite3_stmt   *pstmt;
	int             ret;
	int             retf;

	assert(this);
	/* Check if a key is expected to be generated or not */
	if (this->mob_id == 0)
	{
		q = "INSERT INTO pie_mob (mob_parent_mob_id,mob_name,mob_capture_ts_millis,mob_added_ts_millis,mob_format,mob_color,mob_rating) VALUES (?,?,?,?,?,?,?)";
	}
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	if (this->mob_id == 0)
	{
		ret = sqlite3_bind_int64(pstmt, 1, this->mob_parent_mob_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 2, this->mob_name, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int64(pstmt, 3, this->mob_capture_ts_millis);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int64(pstmt, 4, this->mob_added_ts_millis);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 5, (int) this->mob_format);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 6, (int) this->mob_color);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 7, (int) this->mob_rating);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
	}
	else
	{
		ret = sqlite3_bind_int64(pstmt, 1, this->mob_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int64(pstmt, 2, this->mob_parent_mob_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 3, this->mob_name, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int64(pstmt, 4, this->mob_capture_ts_millis);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int64(pstmt, 5, this->mob_added_ts_millis);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 6, (int) this->mob_format);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 7, (int) this->mob_color);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 8, (int) this->mob_rating);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
	}
	ret = sqlite3_step(pstmt);
	if (ret != SQLITE_DONE)
	{
		ret = -1;
		goto cleanup;
	}
	if (this->mob_id == 0)
	{
		/* Extract last generated key. */
		/* I repeat, this is *NOT* thread safe. */
		this->mob_id = (long) sqlite3_last_insert_rowid(db);
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
pie_mob_read(sqlite3 * db, struct pie_mob * this)
{
	char           *q = "SELECT mob_parent_mob_id,mob_name,mob_capture_ts_millis,mob_added_ts_millis,mob_format,mob_color,mob_rating FROM pie_mob WHERE mob_id = ?";
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
	ret = sqlite3_bind_int64(pstmt, 1, this->mob_id);
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
	this->mob_parent_mob_id = sqlite3_column_int64(pstmt, 0);
	/* Force reading text into memory, and ge the length */
	/* of the string (null terminator not included). */
	/* Allocate memory and copy string to destination, */
	/* and set the null terminator., */
	c = sqlite3_column_text(pstmt, 1);
	br = sqlite3_column_bytes(pstmt, 1);
	this->mob_name = malloc(br + 1);
	memcpy(this->mob_name, c, br);
	this->mob_name[br] = '\0';
	this->mob_capture_ts_millis = sqlite3_column_int64(pstmt, 2);
	this->mob_added_ts_millis = sqlite3_column_int64(pstmt, 3);
	this->mob_format = (short) sqlite3_column_int(pstmt, 4);
	this->mob_color = (char) sqlite3_column_int(pstmt, 5);
	this->mob_rating = (char) sqlite3_column_int(pstmt, 6);
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
pie_mob_update(sqlite3 * db, struct pie_mob * this)
{
	char           *q = "UPDATE pie_mob SET mob_parent_mob_id = ?,mob_name = ?,mob_capture_ts_millis = ?,mob_added_ts_millis = ?,mob_format = ?,mob_color = ?,mob_rating = ? WHERE mob_id = ?";
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
	ret = sqlite3_bind_int64(pstmt, 1, this->mob_parent_mob_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 2, this->mob_name, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int64(pstmt, 3, this->mob_capture_ts_millis);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int64(pstmt, 4, this->mob_added_ts_millis);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 5, (int) this->mob_format);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 6, (int) this->mob_color);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 7, (int) this->mob_rating);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int64(pstmt, 8, this->mob_id);
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
pie_mob_delete(sqlite3 * db, struct pie_mob * this)
{
	char           *q = "DELETE FROM pie_mob WHERE mob_id = ?";
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
	ret = sqlite3_bind_int64(pstmt, 1, this->mob_id);
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
