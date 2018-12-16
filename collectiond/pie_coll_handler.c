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
#include "../lib/s_queue.h"
#include "../encoding/pie_json.h"
#include "../http/pie_util.h"
#include "../dm/pie_collection.h"
#include "../dm/pie_exif_data.h"
#include "../dm/pie_mob.h"
#include "../dm/pie_dev_params.h"
#include "../dm/pie_collection_member.h"
#include "../dm/pie_storage.h"
#include "../dm/pie_host.h"
#include "../jsmn/jsmn.h"
#include "../http/pie_util.h"
#include "../http/pie_http_types.h"
#include "../doml/pie_doml_mob.h"
#include "../mq_msg/pie_mq_msg.h"

extern struct q_producer* export_q;

/**
 * Given a path like /abc/123 extract 123 and store in provided pie_id.
 * @param pie_id to store extracted number in.
 * @param url to parse.
 * @return 0 if pie_id could be extracted, non-zero otherwise.
 */
static int get_id1(pie_id*, const char*);

/**
 * Given a path like /abc/123/def/456 extract 123 and 456 and store
 *  in provided pie_ids.
 * @param pie_id to store first extracted number in.
 * @param pie_id to store second extracted number in.
 * @param url to parse.
 * @return 0 if pie_id could be extracted, non-zero otherwise.
 */
static int get_id2(pie_id*, pie_id*, const char*);

/**
 * Given a path like /abc/123/\S+ extract 123 and '/\S+'.
 * @param pie_id to store extracted number in.
 * @param pointer to store string in
 * @param url to parse
 * @return 0 if parsing was successful.
 */
static int get_id1_path(pie_id*, const char**, const char*);

/**
 * Utility method to read a collection and its assets, and then JSON
 * serialize it.
 * @param buffer to write output.
 * @param pointer to size of buffer. If successfull referenced value
 *        will be updated with the number of bytes written.
 * @param id of collection.
 * @return 0 if sucessfull, otherwise matching HTTP status code.
 */
static int coll_to_json(char*, size_t*, sqlite3*, pie_id);

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

/**
 * Move a MOB from one collection to another.
 */
static int pie_coll_h_coll_asset_post(struct pie_coll_h_resp*,
                                     const char*,
                                     sqlite3*);
/**
 * Delete a MOB from a collection.
 * This will also delete the MOB and all MINs.
 */
static int pie_coll_h_coll_asset_del(struct pie_coll_h_resp*,
                                     const char*,
                                     sqlite3*);

int  pie_coll_h_colls(struct pie_coll_h_resp* r,
                      const char* url,
                      enum pie_http_verb verb,
                      struct pie_http_post_data* data,
                      sqlite3* db)
{
        struct llist* cl;
        struct lnode* n;

        (void)url;
        (void)data;

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

int pie_coll_h_coll(struct pie_coll_h_resp* r,
                    const char* url,
                    enum pie_http_verb verb,
                    struct pie_http_post_data* data,
                    sqlite3* db)
{
        pie_id id;
        size_t len = r->wbuf_len;
        int ret;

        (void)data;

        if (verb != PIE_HTTP_VERB_GET)
        {
                r->http_sc = HTTP_STATUS_METHOD_NOT_ALLOWED;
                return 0;
        }

        if (get_id1(&id, url))
        {
                r->http_sc = HTTP_STATUS_BAD_REQUEST;
                return 0;
        }

        ret = coll_to_json(r->wbuf, &len, db, id);

        if (ret == 0)
        {
                r->content_len = len;
                r->http_sc = HTTP_STATUS_OK;
                r->content_type = "application/json; charset=UTF-8";
        }
        else
        {
                r->content_len = 0;
                r->http_sc = ret;

                switch (ret)
                {
                case HTTP_STATUS_SERVICE_UNAVAILABLE:
                case HTTP_STATUS_INTERNAL_SERVER_ERROR:
                        ret = 1;
                default:
                        ret = 0;
                }
        }

        return ret;
}

int pie_coll_h_coll_asset(struct pie_coll_h_resp* r,
                          const char* url,
                          enum pie_http_verb verb,
                          struct pie_http_post_data* data,
                          sqlite3* db)
{
        (void)data;

        switch (verb)
        {
        case PIE_HTTP_VERB_POST:
                return pie_coll_h_coll_asset_post(r, url, db);
        case PIE_HTTP_VERB_DELETE:
                return pie_coll_h_coll_asset_del(r, url, db);
        default:
                break;
        }

        r->http_sc = HTTP_STATUS_METHOD_NOT_ALLOWED;

        return 0;
}

static int pie_coll_h_coll_asset_post(struct pie_coll_h_resp* r,
                                      const char* url,
                                      sqlite3* db)
{
        pie_id mob_id;
        pie_id tgt_col_id;
        struct pie_collection_member src_col;
        int ok;

        if (get_id2(&tgt_col_id, &mob_id, url))
        {
                r->http_sc = HTTP_STATUS_BAD_REQUEST;
                return 0;
        }

        src_col.cmb_mob_id = mob_id;
        ok = pie_collection_member_find_mob(db, &src_col);
        if (ok)
        {
                PIE_ERR("%d", ok);
                r->http_sc = HTTP_STATUS_NOT_FOUND;
                return 0;
        }

        PIE_LOG("Move: %ld, from: %ld to: %ld",
                mob_id,
                src_col.cmb_col_id,
                tgt_col_id);

        if (pie_doml_mob_move(db, tgt_col_id, src_col.cmb_col_id, mob_id))
        {
                PIE_ERR("pie_doml_mob_move");
                r->http_sc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
        }
        else
        {
                /* Return update collection */
                r->http_sc = HTTP_STATUS_OK;
                r->content_len = 0;
                r->wbuf[0] = '\0';
                r->content_type = "text/plain";
        }

        return 0;
}

static int pie_coll_h_coll_asset_del(struct pie_coll_h_resp* r,
                                     const char* url,
                                     sqlite3* db)
{
        struct pie_collection_member cmb;
        int ok;
        int ret;

        if (get_id2(&cmb.cmb_col_id, &cmb.cmb_mob_id, url))
        {
                r->http_sc = HTTP_STATUS_BAD_REQUEST;
                ret = 0;
                goto done;
        }

        PIE_LOG("Collection: %ld, MOB id: %ld", cmb.cmb_col_id, cmb.cmb_mob_id);

        /* Verify asset is part of collection */
        ok = pie_collection_member_read(db, &cmb);
        if (ok < 0)
        {
                PIE_WARN("pie_collection_member_read: %d", ok);
                r->http_sc = HTTP_STATUS_SERVICE_UNAVAILABLE;
                ret = 1;
                goto done;
        }
        if (ok > 0)
        {
                r->http_sc = HTTP_STATUS_NOT_FOUND;
                ret = 0;
                goto done;
        }

        /* Purge MOB, MINs files etc */
        if (pie_doml_mob_delete(db, cmb.cmb_mob_id))
        {
                r->http_sc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
                r->content_type = "application/json; charset=UTF-8";
                r->content_len = 0;
                ret = 1;
                goto done;
        }

        size_t len = r->wbuf_len;
        ret = coll_to_json(r->wbuf, &len, db, cmb.cmb_col_id);

        if (ret == 0)
        {
                r->content_len = len;
                r->http_sc = HTTP_STATUS_OK;
                r->content_type = "application/json; charset=UTF-8";
        }
        else
        {
                r->content_len = 0;
                r->http_sc = ret;

                switch (ret)
                {
                case HTTP_STATUS_SERVICE_UNAVAILABLE:
                case HTTP_STATUS_INTERNAL_SERVER_ERROR:
                        ret = 1;
                default:
                        ret = 0;
                }
        }

done:

        return ret;
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

        (void)data;

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

        (void)data;

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

        (void)data;

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

int pie_coll_h_stgs(struct pie_coll_h_resp* r,
                    const char* url,
                    enum pie_http_verb verb,
                    struct pie_http_post_data* data,
                    sqlite3* db)
{
        if (verb != PIE_HTTP_VERB_GET)
        {
                r->http_sc = HTTP_STATUS_METHOD_NOT_ALLOWED;
                return 0;
        }

        struct llist* stgs = pie_storage_read_all(db);
        struct pie_http_storages_resp resp;

        resp.storages = llist_create();
        for (struct lnode* n = llist_head(stgs); n; n = n->next)
        {
                struct pie_host hst;
                struct pie_storage* stg = n->data;
                struct pie_http_storage_item* i = malloc(sizeof(struct pie_http_storage_item));
                int status;

                hst.hst_id = stg->stg_hst_id;
                status = pie_host_read(db, &hst);
                if (status > 0)
                {
                        PIE_WARN("No entry for host %d", hst.hst_id);
                        goto next_item;
                }
                if (status < 0)
                {
                        PIE_ERR("DB failed when reading host: %d", hst.hst_id);
                        abort();
                }

                i->id = stg->stg_id;
                strncpy(i->name, stg->stg_name, HTTP_HOSTNAME_LEN);
                strncpy(i->type, pie_storage_type(stg->stg_id), HTTP_HOSTNAME_LEN);
                strncpy(i->hostname, hst.hst_name, HTTP_HOSTNAME_LEN);
                strncpy(i->fqdn, hst.hst_fqdn, HTTP_HOSTNAME_LEN);

                llist_pushb(resp.storages, i);
        next_item:
                pie_storage_free(stg);
        }
        llist_destroy(stgs);

        r->content_len = pie_enc_json_http_stg_resp(r->wbuf,
                                                    r->wbuf_len,
                                                    &resp);
        r->http_sc = HTTP_STATUS_OK;
        r->content_type = "application/json; charset=UTF-8";

        struct lnode* n = llist_head(resp.storages);
        while (n)
        {
                free(n->data);
                n = n->next;
        };

        return 0;
}

/*
 * Export images.
 * The POST methods receives one pie_http_export request which contains
 * the mobs to export. For each mob create one pie_mq_export_media msg
 * and place on export queue.
 */
int pie_coll_h_exp(struct pie_coll_h_resp* r,
                   const char* url,
                   enum pie_http_verb verb,
                   struct pie_http_post_data* data,
                   sqlite3* db)
{
#define BUF_LEN 4096
        struct pie_http_export_request req;
        char* buf = NULL;
        const char *p;
        pie_id stg_id;

        switch (verb)
        {
        case PIE_HTTP_VERB_POST:
                break;
        default:
                r->http_sc = HTTP_STATUS_METHOD_NOT_ALLOWED;
                return 0;
        }

        if (data->p == 0)
        {
                r->http_sc = HTTP_STATUS_BAD_REQUEST;
                return 0;
        }

        if (get_id1_path(&stg_id, &p, url))
        {
                r->http_sc = HTTP_STATUS_BAD_REQUEST;
        }

        PIE_LOG("Export to %s@%d", p, stg_id);

        req.mobs = NULL;
        if (pie_dec_json_export_request(&req, data->data))
        {
                r->http_sc = HTTP_STATUS_BAD_REQUEST;
                return 0;
        }

        buf = malloc(BUF_LEN);
        for (struct lnode* n = llist_head(req.mobs); n; n = n->next)
        {
                struct pie_mq_export_media em;
                size_t bw;

                strncpy(em.path, p, PIE_PATH_LEN);
                em.mob_id = (pie_id)n->data;
                em.stg_id = stg_id;
                em.max_x = req.max_x;
                em.max_y = req.max_y;
                em.type = PIE_MQ_EXP_JPG;
                em.quality = 100;
                em.sharpen = req.sharpen;
                em.disable_exif = req.disable_exif;

                PIE_LOG("export %ld", em.mob_id);

                bw = pie_enc_json_mq_export(buf, BUF_LEN, &em);
                if (bw == 0)
                {
                        PIE_ERR("Failed to encode json for mob %ld", em.mob_id);
                }
                else
                {
                        /* make sure null terminator gets written */
                        bw++;
                        if (export_q->send(export_q->this, buf, bw) != bw)
                        {
                                PIE_ERR("Failed to send export job for %ld",
                                        em.mob_id);
                        }
                }
        }
        if (req.mobs)
        {
                llist_destroy(req.mobs);
                req.mobs = NULL;
        }

        if (buf)
        {
                free(buf);
        }

        r->http_sc = HTTP_STATUS_OK;
        r->content_type = "application/json; charset=UTF-8";

        return 0;
}

static int get_id1(pie_id* id, const char* url)
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

static int get_id2(pie_id* id1, pie_id* id2, const char* url)
{
        char* pid;
        char* p;

        *id1 = 0L;
        *id2 = 0L;

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
                if (*p == '/')
                {
                        break;
                }
                if (!isdigit(*p))
                {
                        PIE_WARN("Invalid (1) pie_id: '%s'", pid);
                        return 1;
                }
                p++;
        }
        /* The error check here is actualy not needed,
           but better to be safe than sorry. */
        *id1 = strtol(pid, &p, 10);
        if (pid == p)
        {
                PIE_WARN("Invalid (2) pie_id: '%s'", pid);
                return 1;
        }

        /* p is not /asset/123 */
        pid = strchr(p + 1, '/');
        if (pid == NULL)
        {
                PIE_ERR("Slash (2) dissapeared from requested URL: '%s'", url);
                return 1;
        }

        /* Advance pointer to first char after '/' */
        pid++;
        p = pid;

        while (*p)
        {
                if (!isdigit(*p++))
                {
                        PIE_WARN("Invalid (3) pie_id: '%s'", pid);
                        return 1;
                }
        }
        /* The error check here is actualy not needed,
           but better to be safe than sorry. */
        *id2 = strtol(pid, &p, 10);
        if (pid == p)
        {
                PIE_WARN("Invalid (4) pie_id: '%s'", pid);
                return 1;
        }

        return 0;
}

static int get_id1_path(pie_id* id, const char** path, const char* url)
{
        char buf[64];
        char* pid;
        char* p;

        /* /abc/123/some/random/data/123 */
        /*      111222222222222222222222 */

        pid = strchr(url + 1, '/');
        if (pid == NULL)
        {
                PIE_ERR("Slash dissapeared from request URL: '%s'", url);
        }
        /* Advance to char after '/' */
        pid++;
        p = pid;
        size_t l = 0;
        while (*p)
        {
                if (*p != '/')
                {
                        l++;
                }
                else
                {
                        *path = p;
                        break;
                }
                p++;
        }

        if (l > 63)
        {
                PIE_WARN("To large number in URL: '%s'", url);
                return 1;
        }

        memcpy(buf, pid, l);
        buf[l] = '\0';
        *id = strtol(pid, &p, 10);
        if (pid == p)
        {
                PIE_WARN("Invalid pie_id: '%s'", buf);
                return 1;
        }

        return 0;
}

static int coll_to_json(char* buf, size_t* len, sqlite3* db, pie_id col_id)
{
        struct pie_collection coll;
        struct llist* ml = NULL;
        size_t bw;
        int ret;

        coll.col_id = col_id;
        ret = pie_collection_read(db, &coll);
        if (ret > 0)
        {
                ret = HTTP_STATUS_NOT_FOUND;
                return ret;
        }
        if (ret < 0)
        {
                PIE_WARN("pie_collection_read: %d", ret);
                ret = HTTP_STATUS_SERVICE_UNAVAILABLE;
                goto done;
        }

        ml = pie_collection_find_assets(db, coll.col_id);
        if (ml == NULL)
        {
                ret = HTTP_STATUS_INTERNAL_SERVER_ERROR;
                goto done;
        }
        bw = pie_enc_json_collection(buf,
                                     *len,
                                     &coll,
                                     ml);
        *len = bw;
        ret = 0;
done:
        if (ml)
        {
                struct lnode* n = llist_head(ml);

                while(n)
                {
                        struct pie_collection_asset* asset = n->data;

                        pie_collection_asset_free(asset);
                        n = n->next;
                }

                llist_destroy(ml);
        }

        pie_collection_release(&coll);

        return ret;
}
