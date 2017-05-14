/* Automatically generated at Sun May 14 09:07:18 2017 */
/* Do not edit - things may break. */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pie_collection.h"
#include "../lib/llist.h"

struct pie_collection *
pie_collection_alloc(void)
{
	struct pie_collection *this = malloc(sizeof(struct pie_collection));

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
	if (this->col_path)
	{
		free(this->col_path);
		this->col_path = NULL;
	}
}
int
pie_collection_create(sqlite3 * db, struct pie_collection * this)
{
	char           *q = "INSERT INTO pie_collection (col_id,col_path,col_usr_id,col_grp_id,col_acl) VALUES (?,?,?,?,?)";
	sqlite3_stmt   *pstmt;
	int             ret;
	int             retf;

	assert(this);
	/* Check if a key is expected to be generated or not */
	if (this->col_id == 0)
	{
		q = "INSERT INTO pie_collection (col_path,col_usr_id,col_grp_id,col_acl) VALUES (?,?,?,?)";
	}
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	if (this->col_id == 0)
	{
		ret = sqlite3_bind_text(pstmt, 1, this->col_path, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 2, (int) this->col_usr_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 3, (int) this->col_grp_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 4, (int) this->col_acl);
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
		ret = sqlite3_bind_text(pstmt, 2, this->col_path, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 3, (int) this->col_usr_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 4, (int) this->col_grp_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 5, (int) this->col_acl);
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
	char           *q = "SELECT col_path,col_usr_id,col_grp_id,col_acl FROM pie_collection WHERE col_id = ?";
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
	this->col_path = malloc(br + 1);
	memcpy(this->col_path, c, br);
	this->col_path[br] = '\0';
	this->col_usr_id = (int) sqlite3_column_int(pstmt, 1);
	this->col_grp_id = (int) sqlite3_column_int(pstmt, 2);
	this->col_acl = (int) sqlite3_column_int(pstmt, 3);
	ret = 0;
cleanup:
	retf = sqlite3_finalize(pstmt);
	if (retf != SQLITE_OK)
	{
		ret = -1;
	}
	return ret;
}

struct pie_collection* 
pie_collection_find_path(sqlite3 * db, const char * path)
{
	char           *q = "SELECT col_id,col_usr_id,col_grp_id,col_acl FROM pie_collection WHERE col_path = ?";
        struct pie_collection* retc;
	sqlite3_stmt   *pstmt;
	int             ret;
	int             retf;
	const unsigned char *c;
	int             br;

	assert(path);
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		retc = NULL;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 1, path, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		retc = NULL;
		goto cleanup;
	}
	ret = sqlite3_step(pstmt);
	if (ret == SQLITE_DONE)
	{
		retc = NULL;
		goto cleanup;
	}
	if (ret != SQLITE_ROW)
	{
		retc = NULL;
		goto cleanup;
	}

        retc = pie_collection_alloc();

        retc->col_id = sqlite3_column_int(pstmt, 0);
        retc->col_path = strdup(path);
	retc->col_usr_id = (int) sqlite3_column_int(pstmt, 1);
	retc->col_grp_id = (int) sqlite3_column_int(pstmt, 2);
	retc->col_acl = (int) sqlite3_column_int(pstmt, 3);
cleanup:
	sqlite3_finalize(pstmt);

	return retc;
}

struct llist* pie_collection_find_all(sqlite3 * db)
{
        struct llist* retl = llist_create();
	char           *q = "SELECT col_id,col_path,col_usr_id,col_grp_id,col_acl FROM pie_collection";
	sqlite3_stmt   *pstmt;
	int             ret;
	int             retf;
	const unsigned char *c;
	int             br;

	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		retl = NULL;
		goto cleanup;
	}

        for (;;)
        {
                struct pie_collection* coll;
                ret = sqlite3_step(pstmt);

                if (ret == SQLITE_DONE)
                {
                        break;
                }
                if (ret != SQLITE_ROW)
                {
                        struct lnode* l = llist_head(retl);

                        while (l)
                        {
                                pie_collection_free((struct pie_collection*)l->data);
                                l = l->next;
                        }
                        llist_destroy(retl);
                        retl = NULL;
                        break;
                }

                coll = pie_collection_alloc();
                coll->col_id = sqlite3_column_int(pstmt, 0);
                c = sqlite3_column_text(pstmt, 1);
                br = sqlite3_column_bytes(pstmt, 1);
                coll->col_path = malloc(br + 1);
                memcpy(coll->col_path, c, br);
                coll->col_path[br] = '\0';
                coll->col_usr_id = (int) sqlite3_column_int(pstmt, 2);
                coll->col_grp_id = (int) sqlite3_column_int(pstmt, 3);
                coll->col_acl = (int) sqlite3_column_int(pstmt, 4);

                llist_pushb(retl, coll);
        }

cleanup:
	sqlite3_finalize(pstmt);

        return retl;
}

int 
pie_collection_update(sqlite3 * db, struct pie_collection * this)
{
	char           *q = "UPDATE pie_collection SET col_path = ?,col_usr_id = ?,col_grp_id = ?,col_acl = ? WHERE col_id = ?";
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
	ret = sqlite3_bind_text(pstmt, 1, this->col_path, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 2, (int) this->col_usr_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 3, (int) this->col_grp_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 4, (int) this->col_acl);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 5, (int) this->col_id);
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
