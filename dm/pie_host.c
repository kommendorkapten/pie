/* Automatically generated at Sun Apr  2 19:14:02 2017 */
/* Do not edit - things may break. */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pie_host.h"
#include "../prunt/pie_log.h"

struct pie_host *
pie_host_alloc(void)
{
	struct pie_host *this = malloc(sizeof(struct pie_host));

	this->hst_name[0] = '\0';
	this->hst_fqdn[0] = '\0';
	return this;
}
void
pie_host_free(struct pie_host * this)
{
	assert(this);
	pie_host_release(this);
	free(this);
}
void
pie_host_release(struct pie_host * this)
{
	assert(this);
	this->hst_name[0] = '\0';
	this->hst_fqdn[0] = '\0';
}
int
pie_host_create(sqlite3 * db, struct pie_host * this)
{
	char	       *q = "INSERT INTO pie_host (hst_id,hst_name,hst_fqdn) VALUES (?,?,?)";
	sqlite3_stmt   *pstmt;
	int		ret;
	int		retf;

	assert(this);
	/* Check if a key is expected to be generated or not */
	if (this->hst_id == 0)
	{
		q = "INSERT INTO pie_host (hst_name,hst_fqdn) VALUES (?,?)";
	}
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	if (this->hst_id == 0)
	{
		ret = sqlite3_bind_text(pstmt, 1, this->hst_name, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 2, this->hst_fqdn, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
	}
	else
	{
		ret = sqlite3_bind_int(pstmt, 1, (int) this->hst_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 2, this->hst_name, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 3, this->hst_fqdn, -1, SQLITE_STATIC);
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
	if (this->hst_id == 0)
	{
		/* Extract last generated key. */
		/* I repeat, this is *NOT* thread safe. */
		this->hst_id = (int) sqlite3_last_insert_rowid(db);
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
pie_host_read(sqlite3 * db, struct pie_host * this)
{
	char	       *q = "SELECT hst_name,hst_fqdn FROM pie_host WHERE hst_id = ?";
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
	ret = sqlite3_bind_int(pstmt, 1, (int) this->hst_id);
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
	memcpy(this->hst_name, c, br);
	this->hst_name[br] = '\0';
	/* Force reading text into memory, and ge the length */
	/* of the string (null terminator not included). */
	/* Allocate memory and copy string to destination, */
	/* and set the null terminator., */
	c = sqlite3_column_text(pstmt, 1);
	br = sqlite3_column_bytes(pstmt, 1);
	memcpy(this->hst_fqdn, c, br);
	this->hst_fqdn[br] = '\0';
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
pie_host_find_name(sqlite3 * db, struct pie_host * this)
{
	char	       *q = "SELECT hst_id,hst_fqdn FROM pie_host WHERE hst_name = ?";
	sqlite3_stmt   *pstmt;
	int		ret;
	int		retf;
	const unsigned char *c;
	int		br;

	assert(this);
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		PIE_WARN("sqlite3_prepare");
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 1, this->hst_name, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		PIE_WARN("sqlite3_bind_text");
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
		PIE_WARN("sqlite3_bind_step:not row");
		ret = -1;
		goto cleanup;
	}
	this->hst_id = sqlite3_column_int(pstmt, 0);

	/* Force reading text into memory, and ge the length */
	/* of the string (null terminator not included). */
	/* Allocate memory and copy string to destination, */
	/* and set the null terminator., */
	c = sqlite3_column_text(pstmt, 1);
	br = sqlite3_column_bytes(pstmt, 1);
	memcpy(this->hst_fqdn, c, br);
	this->hst_fqdn[br] = '\0';
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
pie_host_update(sqlite3 * db, struct pie_host * this)
{
	char	       *q = "UPDATE pie_host SET hst_name = ?,hst_fqdn = ? WHERE hst_id = ?";
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
	ret = sqlite3_bind_text(pstmt, 1, this->hst_name, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 2, this->hst_fqdn, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 3, (int) this->hst_id);
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
pie_host_delete(sqlite3 * db, struct pie_host * this)
{
	char	       *q = "DELETE FROM pie_host WHERE hst_id = ?";
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
	ret = sqlite3_bind_int(pstmt, 1, (int) this->hst_id);
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
