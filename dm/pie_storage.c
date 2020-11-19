/* Automatically generated at Sat Apr  8 17:25:59 2017 */
/* Do not edit - things may break. */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pie_storage.h"
#include "../vendor/llist.h"

struct pie_storage *
pie_storage_alloc(void)
{
	struct pie_storage *this = malloc(sizeof(struct pie_storage));

	this->stg_name[0] = '\0';
	return this;
}
void
pie_storage_free(struct pie_storage * this)
{
	assert(this);
	pie_storage_release(this);
	free(this);
}
void
pie_storage_release(struct pie_storage * this)
{
	assert(this);
	this->stg_name[0] = '\0';
}
int
pie_storage_create(sqlite3 * db, struct pie_storage * this)
{
	char           *q = "INSERT INTO pie_storage (stg_id,stg_name,stg_type,stg_hst_id) VALUES (?,?,?,?)";
	sqlite3_stmt   *pstmt;
	int		ret;
	int		retf;

	assert(this);
	/* Check if a key is expected to be generated or not */
	if (this->stg_id == 0)
	{
		q = "INSERT INTO pie_storage (stg_name,stg_type,stg_hst_id) VALUES (?,?,?)";
	}
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	if (this->stg_id == 0)
	{
		ret = sqlite3_bind_text(pstmt, 1, this->stg_name, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 2, (int) this->stg_type);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 3, (int) this->stg_hst_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
	}
	else
	{
		ret = sqlite3_bind_int(pstmt, 1, (int) this->stg_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 2, this->stg_name, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 3, (int) this->stg_type);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 4, (int) this->stg_hst_id);
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
	if (this->stg_id == 0)
	{
		/* Extract last generated key. */
		/* I repeat, this is *NOT* thread safe. */
		this->stg_id = (int) sqlite3_last_insert_rowid(db);
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
pie_storage_read(sqlite3 * db, struct pie_storage * this)
{
	char           *q = "SELECT stg_name,stg_type,stg_hst_id FROM pie_storage WHERE stg_id = ?";
	sqlite3_stmt   *pstmt;
	int             ret;
	int             retf;
	const unsigned char *c;
	int		br;

	assert(this);

	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 1, (int) this->stg_id);
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
	memcpy(this->stg_name, c, br);
	this->stg_name[br] = '\0';
	this->stg_type = (enum pie_storage_type) sqlite3_column_int(pstmt, 1);
	this->stg_hst_id = (int) sqlite3_column_int(pstmt, 2);
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
pie_storage_update(sqlite3 * db, struct pie_storage * this)
{
	char           *q = "UPDATE pie_storage SET stg_name = ?,stg_type = ?,stg_hst_id = ? WHERE stg_id = ?";
	sqlite3_stmt   *pstmt;
	int             ret;
	int		retf;

	assert(this);
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 1, this->stg_name, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 2, (int) this->stg_type);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 3, (int) this->stg_hst_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 4, (int) this->stg_id);
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
pie_storage_delete(sqlite3 * db, struct pie_storage * this)
{
	char           *q = "DELETE FROM pie_storage WHERE stg_id = ?";
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
	ret = sqlite3_bind_int(pstmt, 1, (int) this->stg_id);
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

struct llist* pie_storage_read_all(sqlite3* db)
{
	char           *q = "SELECT stg_id,stg_name,stg_type,stg_hst_id FROM pie_storage";
	sqlite3_stmt   *pstmt;
        struct llist   *retl = llist_create();
	const unsigned char *c;
        int             ret;
	int		br;

	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
                llist_destroy(retl);
                retl = NULL;
                goto cleanup;
	}

        for (;;)
        {
                struct pie_storage* stg;

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
                                pie_storage_free((struct pie_storage*)l->data);
                                l = l->next;
                        }
                        llist_destroy(retl);
                        retl = NULL;
                        break;
                }

                stg = pie_storage_alloc();
                stg->stg_id = sqlite3_column_int(pstmt, 0);
                /* Force reading text into memory, and ge the length */
                /* of the string (null terminator not included). */
                /* Allocate memory and copy string to destination, */
                /* and set the null terminator., */
                c = sqlite3_column_text(pstmt, 1);
                br = sqlite3_column_bytes(pstmt, 1);
                memcpy(stg->stg_name, c, br);
                stg->stg_name[br] = '\0';
                stg->stg_type = (enum pie_storage_type) sqlite3_column_int(pstmt, 2);
                stg->stg_hst_id = (int) sqlite3_column_int(pstmt, 3);

                llist_pushb(retl, stg);
        }

cleanup:
	sqlite3_finalize(pstmt);

	return retl;
}

const char*
pie_storage_type(enum pie_storage_type t)
{
        switch (t)
        {
        case PIE_STG_INVALID:
                return "invalid";
        case PIE_STG_ONLINE:
                return "online";
        case PIE_STG_NEARLINE:
                return "nearline";
        case PIE_STG_THUMB:
                return "thumbnail";
        case PIE_STG_PROXY:
                return "proxy";
        case PIE_STG_EXPORT:
                return "export";
        default:
                return "invalid";
        }
}
