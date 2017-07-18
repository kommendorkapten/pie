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

#include <stddef.h>
#include <ctype.h>
#include <libwebsockets.h>
#include "pie_coll_handler.h"
#include "../pie_id.h"
#include "../pie_log.h"
#include "../lib/llist.h"
#include "../encoding/pie_json.h"
#include "../dm/pie_collection.h"
#include "../dm/pie_exif_data.h"
#include "../dm/pie_mob.h"

int  pie_coll_h_collections(struct pie_coll_h_resp* r,
                            const char* url,
                            sqlite3* db)
{
        struct llist* cl;
        struct lnode* n;

        cl = pie_collection_find_all(db);
        if (cl == NULL)
        {
                return 0;
        }
        r->content_len = pie_json_enc_collection_list(r->wbuf,
                                                      r->wbuf_len,
                                                      cl);

        n = llist_head(cl);
        while (n)
        {
                struct pie_collection* c = n->data;

                pie_collection_free(n->data);
                n = n->next;
        }
        llist_destroy(cl);

        r->content_type = "application/json; charset=UTF-8";
        r->http_sc = 200;

        return 0;
}

int pie_coll_h_collection(struct pie_coll_h_resp* r,
                          const char* url,
                          sqlite3* db)
{
        struct pie_collection coll;
        struct llist* ml;
        struct lnode* n;
        char* id;
        char* p;
        pie_id coll_id;
        int ret;

        id = strchr(url + 1, '/');
        if (id == NULL)
        {
                PIE_ERR("Slash dissapeared from requested URL: '%s'", url);
                r->http_sc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
                return 1;
        }
        /* Advance pointer to frist char after '/' */
        id++;
        p = id;

        while (*p)
        {
                if (!isdigit(*p++))
                {
                        PIE_WARN("Invalid collection: '%s'", id);
                        r->http_sc = HTTP_STATUS_BAD_REQUEST;
                        return 0;
                }
        }
        /* The error check here is actualy not needed,
           but better to be safe than sorry. */
        coll_id = strtol(id, &p, 10);
        if (id == p)
        {
                PIE_WARN("Invalid collection: '%s'", id);
                r->http_sc = HTTP_STATUS_BAD_REQUEST;
                return 0;
        }

        coll.col_id = coll_id;
        ret = pie_collection_read(db, &coll);
        if (ret > 0)
        {
                r->http_sc = HTTP_STATUS_NOT_FOUND;
                return 0;
        }
        if (ret < 0)
        {
                PIE_WARN("pie_collection_read: %d", ret);
                r->http_sc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
                return 1;
        }

        ml = pie_mob_find_collection(db, coll_id);
        if (ml == NULL)
        {
                r->http_sc = HTTP_STATUS_NOT_FOUND;
                return 0;
        }
        r->content_len = pie_json_enc_collection(r->wbuf,
                                                 r->wbuf_len,
                                                 coll_id,
                                                 ml);

        n = llist_head(ml);
        while(n)
        {
                struct pie_mob* mob = n->data;

                pie_mob_free(mob);
                n = n->next;

        }
        llist_destroy(ml);
        pie_collection_release(&coll);

        r->http_sc = 200;
        r->content_type = "application/json; charset=UTF-8";

        return 0;
}

int pie_coll_h_exif(struct pie_coll_h_resp* r,
                    const char* url,
                    sqlite3* db)
{
        struct pie_exif_data exif;
        char* id;
        char* p;
        pie_id mob_id;
        int ret;

        id = strchr(url + 1, '/');
        if (id == NULL)
        {
                PIE_ERR("Slash dissapeared from requested URL: '%s'", url);
                r->http_sc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
                return 1;
        }
        /* Advance pointer to frist char after '/' */
        id++;
        p = id;

        while (*p)
        {
                if (!isdigit(*p++))
                {
                        PIE_WARN("Invalid collection: '%s'", id);
                        r->http_sc = HTTP_STATUS_BAD_REQUEST;
                        return 0;
                }
        }
        /* The error check here is actualy not needed,
           but better to be safe than sorry. */
        mob_id = strtol(id, &p, 10);
        if (id == p)
        {
                PIE_WARN("Invalid collection: '%s'", id);
                r->http_sc = HTTP_STATUS_BAD_REQUEST;
                return 0;
        }

        exif.ped_mob_id = mob_id;
        ret = pie_exif_data_read(db, &exif);
        if (ret > 0)
        {
                r->http_sc = HTTP_STATUS_NOT_FOUND;
                return 0;
        }
        if (ret < 0)
        {
                PIE_WARN("pie_exif_read: %d", ret);
                r->http_sc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
                return 1;
        }

        r->content_len = pie_json_enc_exif(r->wbuf,
                                           r->wbuf_len,
                                           &exif);

        r->http_sc = 200;
        r->content_type = "application/json; charset=UTF-8";

        /* free any allocated data */
        pie_exif_data_release(&exif);

        return 0;
}
