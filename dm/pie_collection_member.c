/* Automatically generated at Tue Jun  6 08:24:38 2017 */
/* Do not edit - things may break. */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pie_collection_member.h"
struct pie_collection_member *
pie_collection_member_alloc(void)
{
	struct pie_collection_member *this = malloc(sizeof(struct pie_collection_member));

	return this;
}
void 
pie_collection_member_free(struct pie_collection_member * this)
{
	assert(this);
	pie_collection_member_release(this);
	free(this);
}
void 
pie_collection_member_release(struct pie_collection_member * this)
{
	assert(this);
}
int 
pie_collection_member_create(sqlite3 * db, struct pie_collection_member * this)
{
	char           *q = "INSERT INTO pie_collection_member (cmb_col_id,cmb_mob_id) VALUES (?,?)";
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
	ret = sqlite3_bind_int64(pstmt, 1, this->cmb_col_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int64(pstmt, 2, this->cmb_mob_id);
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
pie_collection_member_read(sqlite3 * db, struct pie_collection_member * this)
{
	char           *q = "SELECT FROM pie_collection_member WHERE cmb_col_id = ? AND cmb_mob_id = ?";
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
	ret = sqlite3_bind_int64(pstmt, 1, this->cmb_col_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int64(pstmt, 2, this->cmb_mob_id);
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
pie_collection_member_update(sqlite3 * db, struct pie_collection_member * this)
{
	char           *q = "UPDATE pie_collection_member SET WHERE cmb_col_id = ? AND cmb_mob_id = ?";
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
	ret = sqlite3_bind_int64(pstmt, 1, this->cmb_col_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int64(pstmt, 2, this->cmb_mob_id);
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
pie_collection_member_delete(sqlite3 * db, struct pie_collection_member * this)
{
	char           *q = "DELETE FROM pie_collection_member WHERE cmb_col_id = ? AND cmb_mob_id = ?";
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
	ret = sqlite3_bind_int64(pstmt, 1, this->cmb_col_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int64(pstmt, 2, this->cmb_mob_id);
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
