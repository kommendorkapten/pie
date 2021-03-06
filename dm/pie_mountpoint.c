/* Automatically generated at Sun Apr  2 19:14:02 2017 */
/* Do not edit - things may break. */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pie_mountpoint.h"
struct pie_mountpoint *
pie_mountpoint_alloc(void)
{
	struct pie_mountpoint *this = malloc(sizeof(struct pie_mountpoint));

	this->mnt_path[0] = '\0';
	return this;
}
void
pie_mountpoint_free(struct pie_mountpoint * this)
{
	assert(this);
	pie_mountpoint_release(this);
	free(this);
}
void
pie_mountpoint_release(struct pie_mountpoint * this)
{
	assert(this);
	(void) this;

	this->mnt_path[0] = '\0';
}
int
pie_mountpoint_create(sqlite3 * db, struct pie_mountpoint * this)
{
	char	       *q = "INSERT INTO pie_mountpoint (mnt_hst_id,mnt_stg_id,mnt_path) VALUES (?,?,?)";
	sqlite3_stmt   *pstmt;
	int		ret;
	int		retf;

	assert(this);
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 1, (int) this->mnt_hst_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 2, (int) this->mnt_stg_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 3, this->mnt_path, -1, SQLITE_STATIC);
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
pie_mountpoint_read(sqlite3 * db, struct pie_mountpoint * this)
{
	char	       *q = "SELECT mnt_path FROM pie_mountpoint WHERE mnt_hst_id = ? AND mnt_stg_id = ?";
	sqlite3_stmt   *pstmt;
	int		ret;
	int		retf;
	const unsigned char *c;
	int		br;

	assert(this);
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 1, (int) this->mnt_hst_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 2, (int) this->mnt_stg_id);
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
	memcpy(this->mnt_path, c, br);
	this->mnt_path[br] = '\0';
	ret = 0;
cleanup:
	retf = sqlite3_finalize(pstmt);
	if (retf != SQLITE_OK)
	{
		ret = -1;
	}
	return ret;
}

int pie_mountpoint_find_host(sqlite3 * db, struct pie_mountpoint ** this, int hst_id, size_t len)
{
	char*	       q = "SELECT mnt_stg_id, mnt_path FROM pie_mountpoint WHERE mnt_hst_id = ?";
	sqlite3_stmt   *pstmt;
	int		ret;
	int		retf;
	const unsigned char *c;
	int		br;

	assert(this);
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 1, hst_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}

	for (size_t i = 0; i < len; i++)
	{
		ret = sqlite3_step(pstmt);
		if (ret == SQLITE_DONE)
		{
			this[i] = NULL;
			break;
		}
		if (ret != SQLITE_ROW)
		{
			this[i] = NULL;
			ret = -1;
			goto cleanup;
		}
		this[i] = pie_mountpoint_alloc();
		this[i]->mnt_hst_id = hst_id;
		this[i]->mnt_stg_id = (int) sqlite3_column_int(pstmt, 0);
		/* Force reading text into memory, and ge the length */
		/* of the string (null terminator not included). */
		/* Allocate memory and copy string to destination, */
		/* and set the null terminator., */
		c = sqlite3_column_text(pstmt, 1);
		br = sqlite3_column_bytes(pstmt, 1);
		memcpy(this[i]->mnt_path, c, br);
		this[i]->mnt_path[br] = '\0';
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
pie_mountpoint_update(sqlite3 * db, struct pie_mountpoint * this)
{
	char	       *q = "UPDATE pie_mountpoint SET mnt_path = ? WHERE mnt_hst_id = ? AND mnt_stg_id = ?";
	sqlite3_stmt   *pstmt;
	int		ret;
	int		retf;

	assert(this);
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 1, this->mnt_path, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 2, (int) this->mnt_hst_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 3, (int) this->mnt_stg_id);
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
pie_mountpoint_delete(sqlite3 * db, struct pie_mountpoint * this)
{
	char	       *q = "DELETE FROM pie_mountpoint WHERE mnt_hst_id = ? AND mnt_stg_id = ?";
	sqlite3_stmt   *pstmt;
	int		ret;
	int		retf;

	assert(this);
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 1, (int) this->mnt_hst_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 2, (int) this->mnt_stg_id);
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
