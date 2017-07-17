/*
* Copyright (C) 2016 Fredrik Skogman, skogman - at - gmail.com.
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

#include <stdio.h>
#include "pie_json.h"
#include "../pie_types.h"
#include "../dm/pie_exif_data.h"
#include "../dm/pie_collection.h"
#include "../dm/pie_mob.h"
#include "../lib/llist.h"

/*
Encode to following structure (without newlines)
{
    "l": [1,2,3],
    "r": [1,2,3],
    "g": [1,2,3],
    "b": [1,2,3]
}
 */

size_t pie_json_enc_hist(char* buf, size_t len, const struct pie_histogram* h)
{
        size_t bw = 0;

        bw += snprintf(buf + bw, len - bw, "{\"l\":[%d", h->lum[0]);
        for (int i = 1; i < PIE_HIST_RES; i++)
        {
                bw += snprintf(buf + bw, len - bw, ",%d", h->lum[i]);
        }
        bw += snprintf(buf + bw, len - bw, "],\"r\":[%d", h->c_red[0]);
        for (int i = 1; i < PIE_HIST_RES; i++)
        {
                bw += snprintf(buf + bw, len - bw, ",%d", h->c_red[i]);
        }
        bw += snprintf(buf + bw, len - bw, "],\"g\":[%d", h->c_green[0]);
        for (int i = 1; i < PIE_HIST_RES; i++)
        {
                bw += snprintf(buf + bw, len - bw, ",%d", h->c_green[i]);
        }
        bw += snprintf(buf + bw, len - bw, "],\"b\":[%d", h->c_blue[0]);
        for (int i = 1; i < PIE_HIST_RES; i++)
        {
                bw += snprintf(buf + bw, len - bw, ",%d", h->c_blue[i]);
        }
        bw += snprintf(buf + bw, len - bw, "]}");

        return bw;
}

size_t pie_json_enc_exif(char* buf,
                         size_t len,
                         const struct pie_exif_data* ped)
{
        (void)buf;
        (void)len;
        (void)ped;
        int bw = 0;
        
        return bw;
}

size_t pie_json_enc_collection(char* buf,
                               size_t len,
                               pie_id id,
                               struct llist* ml)
{
        struct lnode* n = llist_head(ml);
        size_t bw = 0;
        int first = 1;

        bw += snprintf(buf + bw, len - bw, "{");
        bw += snprintf(buf + bw, len - bw, "\"id\": \"%ld\",", id);
        bw += snprintf(buf + bw, len - bw, "\"assets\":[");
        while (n)
        {
                struct pie_mob* m = n->data;

                if (!first)
                {
                        bw += snprintf(buf + bw, len - bw, ",");
                }
                else
                {
                        first = 0;
                }

                bw += snprintf(buf + bw,
                               len - bw,
                               "{\"id\": \"%ld\",\"mob\":",
                               m->mob_id);
                bw += pie_json_enc_mob(buf + bw, len - bw, m);
                bw += snprintf(buf + bw, len - bw, "}");
                n = n->next;
        }
        bw += snprintf(buf + bw, len - bw, "]");
        bw += snprintf(buf + bw, len - bw, "}");

        return bw;        
}

size_t pie_json_enc_collection_list(char* buf,
                                    size_t len,
                                    struct llist* cl)
{
        struct lnode* n = llist_head(cl);
        size_t bw = 0;
        int first = 1;

        bw += snprintf(buf + bw, len - bw, "[");
        
        while (n)
        {
                struct pie_collection* c = n->data;

                if (!first)
                {
                        bw += snprintf(buf + bw, len - bw, ",");
                }
                else
                {
                        first = 0;
                }
                bw += snprintf(buf + bw, len - bw, "{\"id\":\"%ld\",\"path\":\"%s\"}",
                       c->col_id,
                       c->col_path);
                
                n = n->next;
        }

        bw += snprintf(buf + bw, len - bw, "]");

        return bw;
}

size_t pie_json_enc_mob(char* buf, size_t len, struct pie_mob* mob)
{
        size_t bw;

        bw = snprintf(buf, len,
                      "{\"id\":\"%ld\"," \
                      "\"parent_id\":\"%ld\"," \
                      "\"name\":\"%s\"," \
                      "\"capture_ts_ms\":%ld,"  \
                      "\"added_ts_ms\":%ld," \
                      "\"format\":\"%d\"," \
                      "\"color\":\"%d\"," \
                      "\"rating\":\"%d\"}",
                      mob->mob_id,
                      mob->mob_parent_mob_id,
                      mob->mob_name,
                      mob->mob_capture_ts_millis,
                      mob->mob_added_ts_millis,
                      mob->mob_format,
                      mob->mob_color,
                      mob->mob_rating);

        return bw;
}
