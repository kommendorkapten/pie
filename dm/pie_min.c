/* Automatically generated at Tue Jun  6 08:53:08 2017 */
/* Do not edit - things may break. */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../lib/llist.h"
#include "pie_min.h"

struct pie_min *
pie_min_alloc(void)
{
	struct pie_min *this = malloc(sizeof(struct pie_min));

	this->min_path[0] = '\0';
	this->min_sha1_hash[0] = '\0';
	return this;
}
void 
pie_min_free(struct pie_min * this)
{
	assert(this);
	pie_min_release(this);
	free(this);
}
void 
pie_min_release(struct pie_min * this)
{
	assert(this);
	this->min_path[0] = '\0';
	this->min_sha1_hash[0] = '\0';
}
int 
pie_min_create(sqlite3 * db, struct pie_min * this)
{
	char           *q = "INSERT INTO pie_min (min_id,min_mob_id,min_added_ts_millis,min_stg_id,min_path,min_sha1_hash) VALUES (?,?,?,?,?,?)";
	sqlite3_stmt   *pstmt;
	int             ret;
	int             retf;

	assert(this);
	/* Check if a key is expected to be generated or not */
	if (this->min_id == 0)
	{
		q = "INSERT INTO pie_min (min_mob_id,min_added_ts_millis,min_stg_id,min_path,min_sha1_hash) VALUES (?,?,?,?,?)";
	}
	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	if (this->min_id == 0)
	{
		ret = sqlite3_bind_int64(pstmt, 1, this->min_mob_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int64(pstmt, 2, this->min_added_ts_millis);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 3, (int) this->min_stg_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 4, this->min_path, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 5, this->min_sha1_hash, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
	}
	else
	{
		ret = sqlite3_bind_int64(pstmt, 1, this->min_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int64(pstmt, 2, this->min_mob_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int64(pstmt, 3, this->min_added_ts_millis);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_int(pstmt, 4, (int) this->min_stg_id);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 5, this->min_path, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK)
		{
			ret = -1;
			goto cleanup;
		}
		ret = sqlite3_bind_text(pstmt, 6, this->min_sha1_hash, -1, SQLITE_STATIC);
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
	if (this->min_id == 0)
	{
		/* Extract last generated key. */
		/* I repeat, this is *NOT* thread safe. */
		this->min_id = (long) sqlite3_last_insert_rowid(db);
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
pie_min_read(sqlite3 * db, struct pie_min * this)
{
	char           *q = "SELECT min_mob_id,min_added_ts_millis,min_stg_id,min_path,min_sha1_hash FROM pie_min WHERE min_id = ?";
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
	ret = sqlite3_bind_int64(pstmt, 1, this->min_id);
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
	this->min_mob_id = sqlite3_column_int64(pstmt, 0);
	this->min_added_ts_millis = sqlite3_column_int64(pstmt, 1);
	this->min_stg_id = (int) sqlite3_column_int(pstmt, 2);
	/* Force reading text into memory, and ge the length */
	/* of the string (null terminator not included). */
	/* Allocate memory and copy string to destination, */
	/* and set the null terminator., */
	c = sqlite3_column_text(pstmt, 3);
	br = sqlite3_column_bytes(pstmt, 3);
	memcpy(this->min_path, c, br);
	this->min_path[br] = '\0';
        /* sha1 hash */
        c = sqlite3_column_text(pstmt, 4);
        br = sqlite3_column_bytes(pstmt, 4);
        memcpy(this->min_sha1_hash, c, br);
        this->min_sha1_hash[br] = '\0';
        
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
pie_min_update(sqlite3 * db, struct pie_min * this)
{
	char           *q = "UPDATE pie_min SET min_mob_id = ?,min_added_ts_millis = ?,min_stg_id = ?,min_path = ?,min_sha1_hash = ? WHERE min_id = ?";
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
	ret = sqlite3_bind_int64(pstmt, 1, this->min_mob_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int64(pstmt, 2, this->min_added_ts_millis);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_int(pstmt, 3, (int) this->min_stg_id);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 4, this->min_path, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}
	ret = sqlite3_bind_text(pstmt, 5, this->min_sha1_hash, -1, SQLITE_STATIC);
	if (ret != SQLITE_OK)
	{
		ret = -1;
		goto cleanup;
	}        
	ret = sqlite3_bind_int64(pstmt, 6, this->min_id);
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
pie_min_delete(sqlite3 * db, struct pie_min * this)
{
	char           *q = "DELETE FROM pie_min WHERE min_id = ?";
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
	ret = sqlite3_bind_int64(pstmt, 1, this->min_id);
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

struct llist* pie_min_find_mob(sqlite3* db, pie_id mob_id)
{
	char           *q = "SELECT min_id,min_added_ts_millis,min_stg_id,min_path,min_sha1_hash FROM pie_min WHERE min_mob_id = ?";
	sqlite3_stmt   *pstmt;
        struct llist   *retl = llist_create();
	int             ret;
	const unsigned char *c;
	int             br;

	ret = sqlite3_prepare_v2(db, q, -1, &pstmt, NULL);
	if (ret != SQLITE_OK)
	{
                llist_destroy(retl);
                retl = NULL;
		goto cleanup;
	}
	ret = sqlite3_bind_int64(pstmt, 1, mob_id);
	if (ret != SQLITE_OK)
	{
                llist_destroy(retl);
                retl = NULL;
		goto cleanup;
	}

        for (;;)
        {
                struct pie_min* min;

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
                                pie_min_free((struct pie_min*)l->data);
                                l = l->next;
                        }
                        llist_destroy(retl);
                        retl = NULL;
                        break;
                }

                min = pie_min_alloc();
                min->min_id = sqlite3_column_int64(pstmt, 0);
                min->min_mob_id = mob_id;
                min->min_added_ts_millis = sqlite3_column_int64(pstmt, 1);
                min->min_stg_id = (int) sqlite3_column_int(pstmt, 2);
                /* Force reading text into memory, and ge the length */
                /* of the string (null terminator not included). */
                /* Allocate memory and copy string to destination, */
                /* and set the null terminator., */
                c = sqlite3_column_text(pstmt, 3);
                br = sqlite3_column_bytes(pstmt, 3);
                memcpy(min->min_path, c, br);
                min->min_path[br] = '\0';
                /* sha1 hash */
                c = sqlite3_column_text(pstmt, 4);
                br = sqlite3_column_bytes(pstmt, 4);
                memcpy(min->min_sha1_hash, c, br);
                min->min_sha1_hash[br] = '\0';

                llist_pushb(retl, min);                
        }

cleanup:
        sqlite3_finalize(pstmt);

	return retl;
}
