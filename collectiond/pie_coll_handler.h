/*
* Copyright (C) 2017 Fredrik Skogman, skogman - at - gmail.com.
* This file is part of pie project
*
* The contents of this file are subject to the terms of the Common
* Development and Distribution License (the "License"). You may not use this
* file except in compliance with the License. You can obtain a copy of the
* License at http://opensource.org/licenses/CDDL-1.0. See the License for the
* specific language governing permissions and limitations under the License.
* When distributing the software, include this License Header Notice in each
* file and include the License file at http://opensource.org/licenses/CDDL-1.0.
*/

#ifndef __PIE_COLL_HANDLER_H__
#define __PIE_COLL_HANDLER_H__

#include <sqlite3.h>
#include "../pie_id.h"
#include "../http/pie_util.h"

struct pie_http_post_data;

struct pie_coll_h_resp
{
        /* Out: Buffer to write data to */
        char*  wbuf;
        /* Out: Content type of data written to buffer */
        const char* content_type;
        /* In: size of wbuf */
        size_t wbuf_len;
        /* Out: Number of bytes (excluding and NULL terminators) writtern */
        size_t content_len;
        /* Out: HTTP status code */
        int    http_sc;
};

/**
 * GET: Read all collections from the provided database.
 */
extern int pie_coll_h_colls(struct pie_coll_h_resp*,
                            const char*,
                            enum pie_http_verb,
                            struct pie_http_post_data*,
                            sqlite3*);

/**
 * GET: Read a single collection from the provided database.
 */
extern int pie_coll_h_coll(struct pie_coll_h_resp*,
                           const char*,
                           enum pie_http_verb,
                           struct pie_http_post_data*,
                           sqlite3*);

/**
 * PUT: Move a single asset from an existing coll to this.
 * DELETE: Remove a single asset from this collection.
 *         This fill remove the file too.
 */
extern int pie_coll_h_coll_asset(struct pie_coll_h_resp*,
                                 const char*,
                                 enum pie_http_verb,
                                 struct pie_http_post_data*,
                                 sqlite3*);

/**
 *GET: Read a exif data for an asset.
 */
extern int pie_coll_h_exif(struct pie_coll_h_resp*,
                           const char*,
                           enum pie_http_verb,
                           struct pie_http_post_data*,
                           sqlite3*);

/**
 * GET: Read mob meta data for an asset.
 * PUT: Update mob meta data.
 */
extern int pie_coll_h_mob(struct pie_coll_h_resp*,
                          const char*,
                          enum pie_http_verb,
                          struct pie_http_post_data*,
                          sqlite3*);

/**
 * GET: Read development parameters for an asset.
 */
extern int pie_coll_h_devp(struct pie_coll_h_resp*,
                           const char*,
                           enum pie_http_verb,
                           struct pie_http_post_data*,
                           sqlite3*);

/**
 * GET: Get a list of available storages.
 */
extern int pie_coll_h_stgs(struct pie_coll_h_resp*,
                           const char*,
                           enum pie_http_verb,
                           struct pie_http_post_data*,
                           sqlite3*);

/**
 * POST: create an export job.
 */
extern int pie_coll_h_exp(struct pie_coll_h_resp*,
                          const char*,
                          enum pie_http_verb,
                          struct pie_http_post_data*,
                          sqlite3*);

#endif /* __PIE_COLL_HANDLER_H__ */
