/* Automatically generated at Sat Oct 14 19:42:38 2017 */
/* Do not edit - things may break. */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pie_dev_params.h"
struct pie_dev_params *
pie_dev_params_alloc(void)
{
	struct pie_dev_params *this = malloc(sizeof(struct pie_dev_params));

	this->pdp_settings[0] = '\0';
	return this;
}
void
pie_dev_params_free(struct pie_dev_params * this)
{
	assert(this);
	pie_dev_params_release(this);
	free(this);
}
void
pie_dev_params_release(struct pie_dev_params * this)
{
	assert(this);

	this->pdp_settings[0] = '\0';
}
int
pie_dev_params_create(sqlite3 * db, struct pie_dev_params * this)
{
	char           *q = "INSERT INTO pie_dev_params (pdp_mob_id,pdp_settings) VALUES (?,?)";
	sqlite3_stmt   *pstmt;
	int             ret;
	int             retf;

	assert(this);
	/* Check if a key is expected to be generated or not */
	if (this->pdp_mob_id == 0)
	{
		q = "INSERT INTO pie_dev_params (pdp_settings) VALUES (?)";
	}
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	if (this->pdp_mob_id == 0)
	{
		ret = sqlite3_bind_text(pstmt, 1, this->pdp_settings, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
	}
	else
	{
		ret = sqlite3_bind_int64(pstmt, 1, this->pdp_mob_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 2, this->pdp_settings, -1, SQLITE_STATIC);
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
	if (this->pdp_mob_id == 0)
	{
		/* Extract last generated key. */
		/* I repeat, this is *NOT* thread safe. */
		this->pdp_mob_id = (long) sqlite3_last_insert_rowid(db);
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
pie_dev_params_read(sqlite3 * db, struct pie_dev_params * this)
{
	char           *q = "SELECT pdp_settings FROM pie_dev_params WHERE pdp_mob_id = ?";
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
	ret = sqlite3_bind_int64(pstmt, 1, this->pdp_mob_id);
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
	memcpy(this->pdp_settings, c, br);
	this->pdp_settings[br] = '\0';
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
pie_dev_params_update(sqlite3 * db, struct pie_dev_params * this)
{
	char           *q = "UPDATE pie_dev_params SET pdp_settings = ? WHERE pdp_mob_id = ?";
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
	ret = sqlite3_bind_text(pstmt, 1, this->pdp_settings, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int64(pstmt, 2, this->pdp_mob_id);
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
pie_dev_params_delete(sqlite3 * db, struct pie_dev_params * this)
{
	char           *q = "DELETE FROM pie_dev_params WHERE pdp_mob_id = ?";
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
	ret = sqlite3_bind_int64(pstmt, 1, this->pdp_mob_id);
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

int pie_dev_params_exist(sqlite3* db, long id)
{
	char *q = "SELECT pdp_mob_id FROM pie_dev_params WHERE pdp_mob_id = ?";
	sqlite3_stmt *pstmt;
        int ret;
        int retf;

	assert(db);
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int64(pstmt, 1, id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_step(pstmt);
	if (ret == SQLITE_DONE)
	{
		ret = 0;
		goto cleanup;
	}
	if (ret != SQLITE_ROW)
	{
		ret = -1;
		goto cleanup;
	}

        ret = 1;

cleanup:
	retf = sqlite3_finalize(pstmt);
	if (retf != SQLITE_OK)
	{
		ret = -1;
	}
	return ret;
}
