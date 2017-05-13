/* Automatically generated at Sat May 13 10:43:09 2017 */
/* Do not edit - things may break. */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pie_collection.h"
struct pie_collection *
pie_collection_alloc(void)
{
	struct pie_collection *this = malloc(sizeof(struct pie_collection));

	this->col_name = NULL;
	this->col_path = NULL;
	return this;
}
void 
pie_collection_free(struct pie_collection * this)
{
	assert(this);
	pie_collection_release(this);
	free(this);
}
void 
pie_collection_release(struct pie_collection * this)
{
	assert(this);
	if (this->col_name)
	{
		free(this->col_name);
		this->col_name = NULL;
	}
	if (this->col_path)
	{
		free(this->col_path);
		this->col_path = NULL;
	}
}
int 
pie_collection_create(sqlite3 * db, struct pie_collection * this)
{
	char           *q = "INSERT INTO pie_collection (col_id,col_name,col_path,hst_usr_id,hst_grp_id,hst_acl) VALUES (?,?,?,?,?,?)";
	sqlite3_stmt   *pstmt;
	int             ret;
	int             retf;

	assert(this);
	/* Check if a key is expected to be generated or not */
	if (this->col_id == 0)
	{
		q = "INSERT INTO pie_collection (col_name,col_path,hst_usr_id,hst_grp_id,hst_acl) VALUES (?,?,?,?,?)";
	}
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	if (this->col_id == 0)
	{
		ret = sqlite3_bind_text(pstmt, 1, this->col_name, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 2, this->col_path, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 3, (int) this->hst_usr_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 4, (int) this->hst_grp_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 5, (int) this->hst_acl);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
	}
	else
	{
		ret = sqlite3_bind_int(pstmt, 1, (int) this->col_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 2, this->col_name, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 3, this->col_path, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 4, (int) this->hst_usr_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 5, (int) this->hst_grp_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 6, (int) this->hst_acl);
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
	if (this->col_id == 0)
	{
		/* Extract last generated key. */
		/* I repeat, this is *NOT* thread safe. */
		this->col_id = (int) sqlite3_last_insert_rowid(db);
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
pie_collection_read(sqlite3 * db, struct pie_collection * this)
{
	char           *q = "SELECT col_name,col_path,hst_usr_id,hst_grp_id,hst_acl FROM pie_collection WHERE col_id = ?";
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
	ret = sqlite3_bind_int(pstmt, 1, (int) this->col_id);
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
	this->col_name = malloc(br + 1);
	memcpy(this->col_name, c, br);
	this->col_name[br] = '\0';
	/* Force reading text into memory, and ge the length */
	/* of the string (null terminator not included). */
	/* Allocate memory and copy string to destination, */
	/* and set the null terminator., */
	c = sqlite3_column_text(pstmt, 1);
	br = sqlite3_column_bytes(pstmt, 1);
	this->col_path = malloc(br + 1);
	memcpy(this->col_path, c, br);
	this->col_path[br] = '\0';
	this->hst_usr_id = (int) sqlite3_column_int(pstmt, 2);
	this->hst_grp_id = (int) sqlite3_column_int(pstmt, 3);
	this->hst_acl = (int) sqlite3_column_int(pstmt, 4);
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
pie_collection_update(sqlite3 * db, struct pie_collection * this)
{
	char           *q = "UPDATE pie_collection SET col_name = ?,col_path = ?,hst_usr_id = ?,hst_grp_id = ?,hst_acl = ? WHERE col_id = ?";
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
	ret = sqlite3_bind_text(pstmt, 1, this->col_name, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 2, this->col_path, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 3, (int) this->hst_usr_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 4, (int) this->hst_grp_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 5, (int) this->hst_acl);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 6, (int) this->col_id);
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
pie_collection_delete(sqlite3 * db, struct pie_collection * this)
{
	char           *q = "DELETE FROM pie_collection WHERE col_id = ?";
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
	ret = sqlite3_bind_int(pstmt, 1, (int) this->col_id);
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
