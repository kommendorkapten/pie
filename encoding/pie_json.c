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
#include <string.h>
#include "pie_json.h"
#include "../pie_log.h"
#include "../pie_types.h"
#include "../dm/pie_exif_data.h"
#include "../dm/pie_collection.h"
#include "../dm/pie_mob.h"
#include "../lib/llist.h"
#include "../jsmn/jsmn.h"

#define DEV_SET_SCALE 1000.0f
#define NUM_TOKENS 512
#define FIELD_LEN 1024

static int pie_ctoa(enum pie_channel);
static int pie_dec_json_curve(struct pie_curve*, char*);
static size_t pie_enc_json_curve(char*,
                                 size_t,
                                 const struct pie_curve*,
                                 enum pie_channel,
                                 int);

/*
Encode to following structure (without newlines)
{
    "l": [1,2,3],
    "r": [1,2,3],
    "g": [1,2,3],
    "b": [1,2,3]
}
 */

size_t pie_enc_json_hist(char* buf, size_t len, const struct pie_histogram* h)
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

/*
 * Encodes all floats with scaling of 10000.
 */
size_t pie_enc_json_settings(char* buf,
                             size_t len,
                             const struct pie_dev_settings* d)
{
        size_t bw = 0;

        bw += snprintf(buf+bw, len-bw, "{\"version\":%d,",
                       d->version);
        bw += snprintf(buf+bw, len-bw, "\"colort\":%d,",
                       (int)(d->color_temp * DEV_SET_SCALE));
        bw += snprintf(buf+bw, len-bw, "\"tint\":%d,",
                       (int)(d->tint * DEV_SET_SCALE));
        bw += snprintf(buf+bw, len-bw, "\"expos\":%d,",
                       (int)(d->exposure * DEV_SET_SCALE));
        bw += snprintf(buf+bw, len-bw, "\"contr\":%d,",
                       (int)(d->contrast * DEV_SET_SCALE));
        bw += snprintf(buf+bw, len-bw, "\"highl\":%d,",
                       (int)(d->highlights * DEV_SET_SCALE));
        bw += snprintf(buf+bw, len-bw, "\"shado\":%d,",
                       (int)(d->shadows * DEV_SET_SCALE));
        bw += snprintf(buf+bw, len-bw, "\"white\":%d,",
                       (int)(d->white * DEV_SET_SCALE));
        bw += snprintf(buf+bw, len-bw, "\"black\":%d,",
                       (int)(d->black * DEV_SET_SCALE));

        bw += snprintf(buf+bw, len-bw, "\"clarity\":{\"amount\": %d,",
                       (int)(d->clarity.amount * DEV_SET_SCALE));
        bw += snprintf(buf+bw, len-bw, "\"rad\": %d,",
                       (int)(d->clarity.radius * DEV_SET_SCALE));
        bw += snprintf(buf+bw, len-bw, "\"thresh\": %d},",
                       (int)(d->clarity.threshold * DEV_SET_SCALE));

        bw += snprintf(buf+bw, len-bw, "\"vibra\":%d,",
                       (int)(d->vibrance * DEV_SET_SCALE));
        bw += snprintf(buf+bw, len-bw, "\"satur\":%d,",
                       (int)(d->saturation * DEV_SET_SCALE));
        bw += snprintf(buf+bw, len-bw, "\"rot\":%d,",
                       (int)(d->rotate * DEV_SET_SCALE));

        bw += snprintf(buf+bw, len-bw, "\"sharp\":{\"amount\": %d,",
                       (int)(d->sharpening.amount * DEV_SET_SCALE));
        bw += snprintf(buf+bw, len-bw, "\"rad\": %d,",
                       (int)(d->sharpening.radius * DEV_SET_SCALE));
        bw += snprintf(buf+bw, len-bw, "\"thresh\": %d},",
                       (int)(d->sharpening.threshold * DEV_SET_SCALE));

        bw += pie_enc_json_curve(buf+bw, len-bw, &d->curve_l, PIE_CHANNEL_RGB, 1);
        bw += pie_enc_json_curve(buf+bw, len-bw, &d->curve_r, PIE_CHANNEL_RED, 1);
        bw += pie_enc_json_curve(buf+bw, len-bw, &d->curve_g, PIE_CHANNEL_GREEN, 1);
        bw += pie_enc_json_curve(buf+bw, len-bw, &d->curve_b, PIE_CHANNEL_BLUE, 0);

        bw += snprintf(buf+bw, len-bw, "}");

        return bw;
}

static size_t pie_enc_json_curve(char* buf,
                                 size_t len,
                                 const struct pie_curve* c,
                                 enum pie_channel chan,
                                 int last)
{
        size_t bw = 0;
        int ic = pie_ctoa(chan);
        int first = 1;

        if (ic == 0)
        {
                return bw;
        }

        bw += snprintf(buf+bw, len-bw, "\"curve_%c\":[", ic);

        for (int i = 0; i < c->num_p; i++)
        {
                if (first)
                {
                        first = 0;
                }
                else
                {
                        bw += snprintf(buf+bw, len-bw, ",");
                }

                bw += snprintf(buf+bw, len-bw, "{\"x\":%d,\"y\":%d}",
                               (int)(c->cntl_p[i].x * DEV_SET_SCALE),
                               (int)(c->cntl_p[i].y * DEV_SET_SCALE));
        }

        if (last)
        {
                bw += snprintf(buf+bw, len-bw, "],");
        }
        else
        {
                bw += snprintf(buf+bw, len-bw, "]");
        }


        return bw;
}

int pie_dec_json_settings(struct pie_dev_settings* s, char* buf)
{
        jsmn_parser parser;
        jsmntok_t* tokens = malloc(NUM_TOKENS * sizeof(jsmntok_t));
        int ret;

        jsmn_init(&parser);
        ret = jsmn_parse(&parser,
                         buf,
                         strlen(buf) + 1,
                         tokens,
                         NUM_TOKENS);
        if (ret < 0)
        {
                PIE_WARN("Failed to JSON parse: '%s'\n",
                         buf);
                ret = 1;
                goto done;
        }
        if (tokens[0].type != JSMN_OBJECT)
        {
                PIE_WARN("Broken state in parser");
                ret = 2;
                goto done;
        }

        /* reset version */
        s->version = 0;

        for (int i = 0; i < ret - 1; i++)
        {
                char field[FIELD_LEN];
                char* p = buf + tokens[i + 1].start;
                int len = tokens[i + 1].end - tokens[i + 1].start;

                if (len > FIELD_LEN)
                {
                        field[FIELD_LEN - 1] = '\0';
                        PIE_ERR("Field ('%s') is longer than %d",
                                field,
                                FIELD_LEN);
                        continue;
                }

                memcpy(field, p, len);
                field[len] = '\0';

                if (pie_enc_jsoneq(buf, tokens + i, "version") == 0)
                {
                        s->version = (int)strtol(field, &p, 10);
                        if (field == p)
                        {
                                PIE_WARN("Invalid %s:%s", "version", field);
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "colort") == 0)
                {
                        s->color_temp = (float)strtol(field, &p, 10) / DEV_SET_SCALE;
                        if (field == p)
                        {
                                PIE_WARN("Invalid %s:%s", "color_temp", field);
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "tint") == 0)
                {
                        s->tint = (float)strtol(field, &p, 10) / DEV_SET_SCALE;
                        if (field == p)
                        {
                                PIE_WARN("Invalid tint %s", field);
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "expos") == 0)
                {
                        s->exposure = (float)strtol(field, &p, 10) / DEV_SET_SCALE;
                        if (field == p)
                        {
                                PIE_WARN("Invalid exposure %s",field);
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "contr") == 0)
                {
                        s->contrast = (float)strtol(field, &p, 10) / DEV_SET_SCALE;
                        if (field == p)
                        {
                                PIE_WARN("Invalid contrast %s", field);
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "highl") == 0)
                {
                        s->highlights = (float)strtol(field, &p, 10) / DEV_SET_SCALE;
                        if (field == p)
                        {
                                PIE_WARN("Invalid highlight %s", field);
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "shado") == 0)
                {
                        s->shadows = (float)strtol(field, &p, 10) / DEV_SET_SCALE;
                        if (field == p)
                        {
                                PIE_WARN("Invalid shadows%s", field);
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "white") == 0)
                {
                        s->white = (float)strtol(field, &p, 10) / DEV_SET_SCALE;
                        if (field == p)
                        {
                                PIE_WARN("Invalid white %s", field);
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "black") == 0)
                {
                        s->black = (float)strtol(field, &p, 10) / DEV_SET_SCALE;
                        if (field == p)
                        {
                                PIE_WARN("Invalid black %s", field);
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "clarity") == 0)
                {
                        if (pie_dec_json_unsharp(&s->clarity, field))
                        {
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "vibra") == 0)
                {
                        s->vibrance = (float)strtol(field, &p, 10) / DEV_SET_SCALE;
                        if (field == p)
                        {
                                PIE_WARN("Invalid vibrance %s", field);
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "satur") == 0)
                {
                        s->saturation = (float)strtol(field, &p, 10) / DEV_SET_SCALE;
                        if (field == p)
                        {
                                PIE_WARN("Invalid saturation %s", field);
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "rot") == 0)
                {
                        s->rotate = (float)strtol(field, &p, 10) / DEV_SET_SCALE;
                        if (field == p)
                        {
                                PIE_WARN("Invalid rotate %s", field);
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "sharp") == 0)
                {
                        if (pie_dec_json_unsharp(&s->sharpening, field))
                        {
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "curve_l") == 0)
                {
                        if (pie_dec_json_curve(&s->curve_l, field))
                        {
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "curve_r") == 0)
                {
                        if (pie_dec_json_curve(&s->curve_r, field))
                        {
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "curve_g") == 0)
                {
                        if (pie_dec_json_curve(&s->curve_g, field))
                        {
                                goto done;
                        }
                }
                else if (pie_enc_jsoneq(buf, tokens + i, "curve_b") == 0)
                {
                        if (pie_dec_json_curve(&s->curve_b, field))
                        {
                                goto done;
                        }
                }
        }

        ret = 0;
done:
        free(tokens);
        return ret;
}

int pie_dec_json_unsharp(struct pie_unsharp_param* s, char* buf)
{
        jsmn_parser parser;
        jsmntok_t tokens[16];
        int ret;

        jsmn_init(&parser);
        ret = jsmn_parse(&parser,
                         buf,
                         strlen(buf) + 1,
                         tokens,
                         sizeof(tokens)/sizeof(tokens[0]));
        if (ret < 0)
        {
                PIE_WARN("Failed to JSON parse: '%s'\n",
                         buf);
                return 1;
        }
        if (tokens[0].type != JSMN_OBJECT)
        {
                PIE_WARN("Broken state in parser");
                return 2;
        }

        for (int i = 0; i < ret - 1; i++)
        {
                char field[128];
                char* p = buf + tokens[i + 1].start;
                int len = tokens[i + 1].end - tokens[i + 1].start;

                if (len > 128)
                {
                        field[127] = '\0';
                        PIE_WARN("Field '%s' is too long", field);
                        continue;
                }

                memcpy(field, p, len);
                field[len] = '\0';

                if (pie_enc_jsoneq(buf, tokens + i, "amount") == 0)
                {
                        s->amount = (float)strtol(field, &p, 10) / DEV_SET_SCALE;
                        if (field == p)
                        {
                                PIE_WARN("Invalid amount %s", field);
                                goto done;
                        }
                }
                else if(pie_enc_jsoneq(buf, tokens + i, "rad") == 0)
                {
                        s->radius = (float)strtol(field, &p, 10) / DEV_SET_SCALE;
                        if (field == p)
                        {
                                PIE_WARN("Invalid radius %s", field);
                                goto done;
                        }
                }
                else if(pie_enc_jsoneq(buf, tokens + i, "thresh") == 0)
                {
                        s->threshold = (float)strtol(field, &p, 10) / DEV_SET_SCALE;
                        if (field == p)
                        {
                                PIE_WARN("Invalid threshold %s", field);
                                goto done;
                        }
                }
        }

        ret = 0;
done:
        return ret;
}

int pie_dec_json_curve(struct pie_curve* s, char* buf)
{
        jsmn_parser parser;
        jsmntok_t* tokens = malloc(NUM_TOKENS * sizeof(jsmntok_t));
        int ret;

        jsmn_init(&parser);
        ret = jsmn_parse(&parser,
                         buf,
                         strlen(buf) + 1,
                         tokens,
                         NUM_TOKENS);
        if (ret < 0)
        {
                PIE_WARN("Failed to JSON parse: '%s'", buf);
                ret = 1;
                goto done;
        }
        if (tokens[0].type != JSMN_ARRAY)
        {
                PIE_WARN("Broken state in parser");
                ret = 2;
                goto done;
        }

        s->num_p = 0;
        for (int i = 1; i < ret; i++) {
                jsmn_parser cparser;
                jsmntok_t ctokens[16];
                char* p = buf + tokens[i].start;
                int len = tokens[i].end - tokens[i].start;
                int cret;

                if (tokens[i].type != JSMN_OBJECT)
                {
                        continue;
                }

                jsmn_init(&cparser);
                cret = jsmn_parse(&cparser,
                                  p,
                                  len,
                                  ctokens,
                                  sizeof(ctokens)/sizeof(ctokens[0]));

                if (cret < 0)
                {
                        PIE_WARN("Failed to JSON parse '%s'", buf);
                        ret = 3;
                        goto done;
                }

                if (ctokens[0].type != JSMN_OBJECT)
                {
                        PIE_WARN("Broken state in parser");
                        ret = 4;
                        goto done;
                }

                if (s->num_p == PIE_CURVE_MAX_CNTL_P)
                {
                        PIE_WARN("To many control points in curve");
                        ret = 5;
                        goto done;
                }

                for (int j = 0; j < cret - 1; j++)
                {
                        char field[64];
                        char* cp = p + ctokens[j + 1].start;
                        len = ctokens[j + 1].end - ctokens[j + 1].start;

                        if (len > 64)
                        {
                                field[64 - 1] = '\0';
                                PIE_WARN("Field '%s' is too long", field);
                                continue;
                        }

                        memcpy(field, cp, len);
                        field[len] = '\0';

                        if (pie_enc_jsoneq(p, ctokens + j, "x") == 0)
                        {
                                s->cntl_p[s->num_p].x = (float)strtol(field, &cp, 10) / DEV_SET_SCALE;
                                if (field == cp)
                                {
                                        PIE_WARN("Invalid x %s", field);
                                        goto done;
                                }
                        }
                        if (pie_enc_jsoneq(p, ctokens + j, "y") == 0)
                        {
                                s->cntl_p[s->num_p].y = (float)strtol(field, &cp, 10) / DEV_SET_SCALE;
                                if (field == cp)
                                {
                                        PIE_WARN("Invalid y %s", field);
                                        goto done;
                                }
                        }
                }
                s->num_p++;
        }

        ret = 0;
done:
        free(tokens);
        return ret;
}

size_t pie_enc_json_exif(char* buf,
                         size_t len,
                         const struct pie_exif_data* ped)
{
        int bw = 0;

        bw += snprintf(buf + bw, len - bw, "{\"id\":\"%ld\",", ped->ped_mob_id);
        bw += snprintf(buf + bw, len - bw, "\"artist\":\"%s\",", ped->ped_artist);
        bw += snprintf(buf + bw, len - bw, "\"copyright\":\"%s\",", ped->ped_copyright);
        bw += snprintf(buf + bw, len - bw, "\"software\":\"%s\",", ped->ped_software);
        bw += snprintf(buf + bw, len - bw, "\"date\":\"%s\",", ped->ped_date_time);
        bw += snprintf(buf + bw, len - bw, "\"lens\":\"%s\",", ped->ped_lens_model);
        bw += snprintf(buf + bw, len - bw, "\"make\":\"%s\",", ped->ped_make);
        bw += snprintf(buf + bw, len - bw, "\"model\":\"%s\",", ped->ped_model);
        bw += snprintf(buf + bw, len - bw, "\"exposure_time\":\"%s\",", ped->ped_exposure_time);
        bw += snprintf(buf + bw, len - bw, "\"sub_sec_time\":%d,", ped->ped_sub_sec_time);
        bw += snprintf(buf + bw, len - bw, "\"x\":%d,", ped->ped_x_dim);
        bw += snprintf(buf + bw, len - bw, "\"y\":%d,", ped->ped_y_dim);
        bw += snprintf(buf + bw, len - bw, "\"iso\":%d,", ped->ped_iso);
        bw += snprintf(buf + bw, len - bw, "\"gamma\":%d,", ped->ped_gamma);
        bw += snprintf(buf + bw, len - bw, "\"white_point\":%d,", ped->ped_white_point);
        bw += snprintf(buf + bw, len - bw, "\"orientation\":%d,", ped->ped_orientation);
        bw += snprintf(buf + bw, len - bw, "\"focal_len\":%d,", ped->ped_focal_len);
        bw += snprintf(buf + bw, len - bw, "\"fnumber\":%d,", ped->ped_fnumber);
        bw += snprintf(buf + bw, len - bw, "\"exposure_bias\":%d,", ped->ped_exposure_bias);
        bw += snprintf(buf + bw, len - bw, "\"white_balance\":%d,", ped->ped_white_balance);
        bw += snprintf(buf + bw, len - bw, "\"exposure_prog\":%d,", ped->ped_exposure_prog);
        bw += snprintf(buf + bw, len - bw, "\"metering_mode\":%d,", ped->ped_metering_mode);
        bw += snprintf(buf + bw, len - bw, "\"flash\":%d,", ped->ped_flash);
        bw += snprintf(buf + bw, len - bw, "\"exposure_mode\":%d,", ped->ped_exposure_mode);
        bw += snprintf(buf + bw, len - bw, "\"color_space\":%d}", ped->ped_color_space);

        /* Remove any control characters */
        for (int i = 0; i < bw; i++)
        {
                switch (buf[i])
                {
                case '\b':
                case '\f':
                case '\n':
                case '\r':
                case '\t':
                case '\\':
                        buf[i] = ' ';
                        break;
                }
        }

        return bw;
}

size_t pie_enc_json_collection(char* buf,
                               size_t len,
                               const struct pie_collection* c,
                               struct llist* ml)
{
        struct lnode* n = llist_head(ml);
        size_t bw = 0;
        int first = 1;

        bw += snprintf(buf + bw, len - bw, "{");
        bw += snprintf(buf + bw, len - bw, "\"id\":\"%ld\",\"path\":\"%s\",",
                       c->col_id,
                       c->col_path);
        bw += snprintf(buf + bw, len - bw, "\"assets\":[");
        while (n)
        {
                struct pie_collection_asset* a = n->data;

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
                               "{\"id\": \"%ld\",\"developed\":%d,\"mob\":",
                               a->mob->mob_id,
                               a->developed);
                bw += pie_enc_json_mob(buf + bw, len - bw, a->mob);
                bw += snprintf(buf + bw, len - bw, "}");
                n = n->next;
        }
        bw += snprintf(buf + bw, len - bw, "]");
        bw += snprintf(buf + bw, len - bw, "}");

        return bw;
}

size_t pie_enc_json_collection_list(char* buf,
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
                bw += snprintf(buf + bw, len - bw,
                               "{\"id\":\"%ld\",\"path\":\"%s\",\"usr_id\":\"%d\",\"grp_id\":\"%d\",\"acl\":\"%d\",\"count\":%d}",
                               c->col_id,
                               c->col_path,
                               c->col_usr_id,
                               c->col_grp_id,
                               c->col_acl,
                               c->col_count);

                n = n->next;
        }

        bw += snprintf(buf + bw, len - bw, "]");

        return bw;
}

size_t pie_enc_json_mob(char* buf, size_t len, const struct pie_mob* mob)
{
        size_t bw;

        bw = snprintf(buf, len,
                      "{\"id\":\"%ld\"," \
                      "\"parent_id\":\"%ld\"," \
                      "\"name\":\"%s\"," \
                      "\"capture_ts_ms\":%ld,"  \
                      "\"added_ts_ms\":%ld," \
                      "\"format\":\"%d\"," \
                      "\"color\":%d," \
                      "\"rating\":%d," \
                      "\"orientation\":%d}",
                      mob->mob_id,
                      mob->mob_parent_mob_id,
                      mob->mob_name,
                      mob->mob_capture_ts_millis,
                      mob->mob_added_ts_millis,
                      mob->mob_format,
                      mob->mob_color,
                      mob->mob_rating,
                      mob->mob_orientation);

        return bw;
}

/* Stolen from jsmn/example/simple.c */
int pie_enc_jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
        if (tok->type == JSMN_STRING &&
            (int) strlen(s) == tok->end - tok->start &&
            strncmp(json + tok->start, s, tok->end - tok->start) == 0)
        {
                return 0;
        }
        return -1;
}

static int pie_ctoa(enum pie_channel channel)
{
        int c = 0;

        switch (channel)
        {
        case PIE_CHANNEL_RED:
                c = 'r';
                break;
        case PIE_CHANNEL_GREEN:
                c = 'g';
                break;
        case PIE_CHANNEL_BLUE:
                c = 'b';
                break;
        case PIE_CHANNEL_RGB:
                c = 'l';
                break;
        default:
                break;
        }

        return c;
}
