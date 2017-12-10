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
#include "../http/pie_util.h"
#include "../dm/pie_collection.h"
#include "../dm/pie_exif_data.h"
#include "../dm/pie_mob.h"
#include "../dm/pie_dev_params.h"
#include "../jsmn/jsmn.h"
#include "../http/pie_util.h"

/**
 * Given a path like /abc/123 extract 123 and store in provided pie_id.
 * @param pie_id to store extracted number in.
 * @param url to parse.
 * @return 0 if pie_id could be extracted, non-zero otherwise.
 */
int get_id1(pie_id*, const char*);

/**
 * Given a path like /abc/123/def/456 extract 123 and 456 and store
 *  in provided pie_ids.
 * @param pie_id to store first extracted number in.
 * @param pie_id to store second extracted number in.
 * @param url to parse.
 * @return 0 if pie_id could be extracted, non-zero otherwise.
 */
int get_id2(pie_id*, pie_id*, const char*);


static int pie_coll_h_exif_get(struct pie_coll_h_resp*,
                               const char*,
                               struct pie_http_post_data*,
                               sqlite3*);

static int pie_coll_h_exif_put(struct pie_coll_h_resp*,
                               const char*,
                               struct pie_http_post_data*,
                               sqlite3*);

static int pie_coll_h_mob_get(struct pie_coll_h_resp*,
                              const char*,
                              struct pie_http_post_data*,
                              sqlite3*);

static int pie_coll_h_mob_put(struct pie_coll_h_resp*,
                              const char*,
                              struct pie_http_post_data*,
                              sqlite3*);

int  pie_coll_h_collections(struct pie_coll_h_resp* r,
                            const char* url,
                            enum pie_http_verb verb,
                            struct pie_http_post_data* data,
                            sqlite3* db)
{
        struct llist* cl;
        struct lnode* n;

        (void)url;

        if (verb != PIE_HTTP_VERB_GET)
        {
                r->http_sc = HTTP_STATUS_METHOD_NOT_ALLOWED;
                return 0;
        }

        cl = pie_collection_find_all(db);
        if (cl == NULL)
        {
                return 0;
        }

        /* Fetch cardinality for each collection */
        n = llist_head(cl);
        while (n)
        {
                if (pie_collection_read_count(db, n->data))
                {
                        PIE_WARN("pie_collection_read_count");
                }
                n = n->next;
        }

        r->content_len = pie_enc_json_collection_list(r->wbuf,
                                                      r->wbuf_len,
                                                      cl);

        n = llist_head(cl);
        while (n)
        {
                pie_collection_free(n->data);
                n = n->next;
        }
        llist_destroy(cl);

        r->content_type = "application/json; charset=UTF-8";
        r->http_sc = HTTP_STATUS_OK;

        return 0;
}

int pie_coll_h_collection(struct pie_coll_h_resp* r,
                          const char* url,
                          enum pie_http_verb verb,
                          struct pie_http_post_data* data,
                          sqlite3* db)
{
        struct pie_collection coll;
        struct llist* ml;
        struct lnode* n;
        int ret;

        if (verb != PIE_HTTP_VERB_GET)
        {
                r->http_sc = HTTP_STATUS_METHOD_NOT_ALLOWED;
                return 0;
        }

        if (get_id1(&coll.col_id, url))
        {
                r->http_sc = HTTP_STATUS_BAD_REQUEST;
                return 0;
        }

        ret = pie_collection_read(db, &coll);
        if (ret > 0)
        {
                r->http_sc = HTTP_STATUS_NOT_FOUND;
                return 0;
        }
        if (ret < 0)
        {
                PIE_WARN("pie_collection_read: %d", ret);
                r->http_sc = HTTP_STATUS_SERVICE_UNAVAILABLE;
                return 1;
        }

        ml = pie_collection_find_assets(db, coll.col_id);
        if (ml == NULL)
        {
                r->http_sc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
                return 0;
        }
        r->content_len = pie_enc_json_collection(r->wbuf,
                                                 r->wbuf_len,
                                                 &coll,
                                                 ml);

        n = llist_head(ml);
        while(n)
        {
                struct pie_collection_asset* asset = n->data;

                pie_collection_asset_free(asset);
                n = n->next;

        }
        llist_destroy(ml);
        pie_collection_release(&coll);

        r->http_sc = HTTP_STATUS_OK;
        r->content_type = "application/json; charset=UTF-8";

        return 0;
}

int pie_coll_h_exif(struct pie_coll_h_resp* r,
                    const char* url,
                    enum pie_http_verb verb,
                    struct pie_http_post_data* data,
                    sqlite3* db)
{
        switch (verb)
        {
        case PIE_HTTP_VERB_GET:
                return pie_coll_h_exif_get(r, url, NULL, db);
        case PIE_HTTP_VERB_PUT:
                return pie_coll_h_exif_put(r, url, data, db);
        default:
                r->http_sc = HTTP_STATUS_METHOD_NOT_ALLOWED;
        }

        return 0;
}

static int pie_coll_h_exif_get(struct pie_coll_h_resp* r,
                               const char* url,
                               struct pie_http_post_data* data,
                               sqlite3* db)
{
        struct pie_exif_data exif;
        int ret;

        if (get_id1(&exif.ped_mob_id, url))
        {
                r->http_sc = HTTP_STATUS_BAD_REQUEST;
                return 0;
        }

        ret = pie_exif_data_read(db, &exif);
        if (ret > 0)
        {
                r->http_sc = HTTP_STATUS_NOT_FOUND;
                return 0;
        }
        if (ret < 0)
        {
                PIE_WARN("pie_exif_read: %d", ret);
                r->http_sc = HTTP_STATUS_SERVICE_UNAVAILABLE;
                return 1;
        }

        r->content_len = pie_enc_json_exif(r->wbuf,
                                           r->wbuf_len,
                                           &exif);

        r->http_sc = HTTP_STATUS_OK;
        r->content_type = "application/json; charset=UTF-8";

        /* free any allocated data */
        pie_exif_data_release(&exif);

        return 0;
}

static int pie_coll_h_exif_put(struct pie_coll_h_resp* r,
                               const char* url,
                               struct pie_http_post_data* data,
                               sqlite3* db)
{
        (void)url;
        (void)data;
        (void)db;

        r->http_sc = 501;
        return 0;
}

int pie_coll_h_mob(struct pie_coll_h_resp* r,
                   const char* url,
                   enum pie_http_verb verb,
                   struct pie_http_post_data* data,
                   sqlite3* db)
{
        switch (verb)
        {
        case PIE_HTTP_VERB_GET:
                return pie_coll_h_mob_get(r, url, NULL, db);
        case PIE_HTTP_VERB_PUT:
                return pie_coll_h_mob_put(r, url, data, db);
        default:
                r->http_sc = HTTP_STATUS_METHOD_NOT_ALLOWED;
        }

        return 0;
}

static int pie_coll_h_mob_get(struct pie_coll_h_resp* r,
                              const char* url,
                              struct pie_http_post_data* data,
                              sqlite3* db)
{
        struct pie_mob mob;
        int ret;

        if (get_id1(&mob.mob_id, url))
        {
                r->http_sc = HTTP_STATUS_BAD_REQUEST;
                return 0;
        }

        ret = pie_mob_read(db, &mob);
        if (ret > 0)
        {
                r->http_sc = HTTP_STATUS_NOT_FOUND;
                return 0;
        }
        if (ret < 0)
        {
                PIE_WARN("pie_mob_read: %d", ret);
                r->http_sc = HTTP_STATUS_SERVICE_UNAVAILABLE;
                return 1;
        }

        r->content_len = pie_enc_json_mob(r->wbuf,
                                          r->wbuf_len,
                                          &mob);

        r->http_sc = HTTP_STATUS_OK;
        r->content_type = "application/json; charset=UTF-8";

        /* free any allocated data */
        pie_mob_release(&mob);

        return 0;
}

static int pie_coll_h_mob_put(struct pie_coll_h_resp* r,
                              const char* url,
                              struct pie_http_post_data* data,
                              sqlite3* db)
{
        struct pie_mob mob;
        jsmn_parser parser;
        jsmntok_t tokens[32]; /* Can only parse with 32 tokens */
        int ret;

        data->data[data->p] = 0;
        PIE_DEBUG("Url: '%s'", url);
        PIE_TRACE("Got mob: '%s'", data->data);

        if (get_id1(&mob.mob_id, url))
        {
                r->http_sc = HTTP_STATUS_BAD_REQUEST;
                goto bailout;
        }
        PIE_DEBUG("Update mob: %ld", mob.mob_id);

        /* Fetch mob from database */
        ret = pie_mob_read(db, &mob);
        if (ret > 0)
        {
                r->http_sc = HTTP_STATUS_NOT_FOUND;
                ret = 0;
                goto release;
        }
        if (ret < 0)
        {
                PIE_WARN("pie_mob_read: %d", ret);
                r->http_sc = HTTP_STATUS_SERVICE_UNAVAILABLE;
                goto bailout;
        }

        /* Parse new mob */
        jsmn_init(&parser);
        ret = jsmn_parse(&parser,
                         data->data,
                         data->p,
                         tokens,
                         sizeof(tokens)/sizeof(tokens[0]));
        if (ret < 0)
        {
                PIE_WARN("Could not parse mob JSON");
                goto bailout;
        }

        if (tokens[0].type != JSMN_OBJECT)
        {
                PIE_WARN("Object epxected when parsing mob JSON");
                goto bailout;
        }

        /* Update new fields.
         * Need to parse the JSON here (which is bad) as we must be able
         * to determine which fields that were updated, so pie_dec_json_mob
         * can not be used. */
        for (int i = 1; i < ret - 1; i++)
        {
                char field[64];
                char* p = data->data + tokens[i + 1].start;
                int token_len = tokens[i + 1].end - tokens[i + 1].start;

                if (token_len > 63)
                {
                        r->http_sc = HTTP_STATUS_REQ_ENTITY_TOO_LARGE;
                        goto bailout;
                }

                memcpy(field, p, token_len);
                field[token_len] = '\0';

                if (pie_enc_jsoneq(data->data, tokens + i, "parent_id") == 0)
                {
                        PIE_DEBUG("parent_id: %s", field);
                        mob.mob_parent_mob_id = strtol(field, &p, 10);
                        if (field == p)
                        {
                                PIE_WARN("Invalid parent id: '%s'", field);
                                goto bailout;
                        }
                }
                else if (pie_enc_jsoneq(data->data, tokens + i, "color") == 0)
                {
                        PIE_DEBUG("color: %s", field);
                        mob.mob_color = (char)strtol(field, &p, 10);
                        if (field == p)
                        {
                                PIE_WARN("Invalid color: '%s'", field);
                                goto bailout;
                        }
                }
                else if (pie_enc_jsoneq(data->data, tokens + i, "rating") == 0)
                {
                        PIE_DEBUG("rating: %s", field);
                        mob.mob_rating = (char)strtol(field, &p, 10);
                        if (field == p)
                        {
                                PIE_WARN("Invalid rating: '%s'", field);
                                goto bailout;
                        }
                }
                else if (pie_enc_jsoneq(data->data, tokens + i, "orientation") == 0)
                {
                        PIE_DEBUG("orientation: %s", field);
                        mob.mob_orientation = (char)strtol(field, &p, 10);
                        if (field == p)
                        {
                                PIE_WARN("Invalid orientation: '%s'", field);
                                goto bailout;
                        }
                }
        }

        /* Store update mob */
        ret = pie_mob_update(db, &mob);
        if (ret > 0)
        {
                r->http_sc = HTTP_STATUS_SERVICE_UNAVAILABLE;
                goto bailout;
        }

        /* Write back result */
        r->content_len = pie_enc_json_mob(r->wbuf,
                                          r->wbuf_len,
                                          &mob);

        r->http_sc = HTTP_STATUS_OK;
        r->content_type = "application/json; charset=UTF-8";

        ret = 0;
        goto release;

bailout:
        ret = -1;

release:
        /* free any allocated data */
        pie_mob_release(&mob);

        return ret;
}

int pie_coll_h_devp(struct pie_coll_h_resp* r,
                    const char* url,
                    enum pie_http_verb verb,
                    struct pie_http_post_data* data,
                    sqlite3* db)
{
        struct pie_dev_params devp;
        int ret;

        if (verb != PIE_HTTP_VERB_GET)
        {
                r->http_sc = HTTP_STATUS_METHOD_NOT_ALLOWED;
                return 0;
        }

        if (get_id1(&devp.pdp_mob_id, url))
        {
                r->http_sc = HTTP_STATUS_BAD_REQUEST;
                return 0;
        }

        ret = pie_dev_params_read(db, &devp);
        if (ret > 0)
        {
                r->http_sc = HTTP_STATUS_NOT_FOUND;
                return 0;
        }
        if (ret < 0)
        {
                PIE_WARN("pie_dev_params_read: %d", ret);
                r->http_sc = HTTP_STATUS_SERVICE_UNAVAILABLE;
                return 1;
        }

        r->content_len = strlen(devp.pdp_settings);
        memcpy(r->wbuf, devp.pdp_settings, r->content_len);

        r->http_sc = HTTP_STATUS_OK;
        r->content_type = "application/json; charset=UTF-8";

        /* free any allocated data */
        pie_dev_params_release(&devp);

        return 0;
}

int get_id1(pie_id* id, const char* url)
{
        char* pid;
        char* p;

        pid = strchr(url + 1, '/');
        if (pid == NULL)
        {
                PIE_ERR("Slash dissapeared from requested URL: '%s'", url);
                return 1;
        }

        /* Advance pointer to frist char after '/' */
        pid++;
        p = pid;

        while (*p)
        {
                if (!isdigit(*p++))
                {
                        PIE_WARN("Invalid pie_id: '%s'", pid);
                        return 1;
                }
        }
        /* The error check here is actualy not needed,
           but better to be safe than sorry. */
        *id = strtol(pid, &p, 10);
        if (pid == p)
        {
                PIE_WARN("Invalid pie_id: '%s'", pid);
                return 1;
        }

        return 0;
}

int get_id2(pie_id* id1, pie_id* id2, const char* url)
{
        printf("parse: %s\n", url);

        *id1 = 0L;
        *id2 = 0L;

        return 0;
}
