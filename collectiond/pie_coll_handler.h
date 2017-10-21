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
 * Read all collections from the provided database.
 * Encode the collections to a JSON list in the provided buffer.
 * @param response struct.
 * @param request URL.
 * @param database to read collections from.
 * @return 0 on sucess.
 */
extern int pie_coll_h_collections(struct pie_coll_h_resp*,
                                  const char*,
                                  sqlite3*);

/**
 * Read a single collection from the provided database.
 * JSON encode the collection.
 * @param response struct.
 * @param request URL.
 * @param database to read collections from.
 * @return 0 on success.
 */
extern int pie_coll_h_collection(struct pie_coll_h_resp*,
                                 const char*,
                                 sqlite3*);

/**
 * Read a exif data for an asset.
 * JSON encode the exif data.
 * @param response struct.
 * @param request URL.
 * @param database to read exif data from.
 * @return 0 on success.
 */
extern int pie_coll_h_exif(struct pie_coll_h_resp*,
                           const char*,
                           sqlite3*);

/**
 * Update exif data for an asset.
 * JSON encode the exif data.
 * @param response struct.
 * @param request URL.
 * @param database handle.
 * @return 0 on success.
 */
extern int pie_coll_h_exif_put(struct pie_coll_h_resp*,
                               const char*,
                               struct pie_http_post_data*,
                               sqlite3*);

/**
 * Read mob meta data for an asset.
 * JSON encode the mob data.
 * @param response struct.
 * @param request URL.
 * @param database handle.
 * @return 0 on success.
 */
extern int pie_coll_h_mob(struct pie_coll_h_resp*,
                          const char*,
                          sqlite3*);

/**
 * Update mob meta data for an asset.
 * JSON encode the mob data.
 * @param response struct.
 * @param request URL.
 * @param database handle.
 * @return 0 on success.
 */
extern int pie_coll_h_mob_put(struct pie_coll_h_resp*,
                              const char*,
                              struct pie_http_post_data*,
                              sqlite3*);

/**
 * Read development parameters for an asset.
 * JSON encode the parameters
 * @param response struct.
 * @param request URL.
 * @param database handle.
 * @return 0 on success.
 */
extern int pie_coll_h_devp(struct pie_coll_h_resp*,
                           const char*,
                           sqlite3*);

#endif /* __PIE_COLL_HANDLER_H__ */
