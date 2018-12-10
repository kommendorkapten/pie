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

#include <stdlib.h>
#include <libwebsockets.h>
#include <string.h>
#include <strings.h>
#include "pie_util.h"
#include "pie_session.h"
#include "../pie_log.h"
#include "../lib/hmap.h"

const char* pie_http_get_mimetype(const char *path)
{
        size_t n = strlen(path);

        if (n < 5)
        {
                return NULL;
        }

        if (strcasecmp(&path[n - 4], ".ico") == 0)
        {
                return "image/x-icon";
        }
        if (strcasecmp(&path[n - 4], ".png") == 0)
        {
                return "image/png";
        }
        if (strcasecmp(&path[n - 5], ".html") == 0)
        {
                return "text/html";
        }
        if (strcasecmp(&path[n - 4], ".css") == 0)
        {
                return "text/css";
        }
        if (strcasecmp(&path[n - 4], ".jpg") == 0)
        {
                return "image/jpeg";
        }
        if (strcasecmp(&path[n - 3], ".js") == 0)
        {
                return "text/javascript";
        }

        return NULL;
}

struct hmap* pie_http_req_params(struct lws* wsi)
{
        char header[256];
        struct hmap* h = hmap_create(NULL, NULL, 8, 0.7f);
        char* p;
        int n = 0;

        while (lws_hdr_copy_fragment(wsi, header, sizeof(header),
                                     WSI_TOKEN_HTTP_URI_ARGS, n++) > 0) {
                header[255] = 0;
                p = strchr(header, '=');
                if (p == NULL)
                {
                        continue;
                }
                ptrdiff_t key_len = p - &header[0];
                size_t val_len = strlen(p + 1);
                char* key = malloc(key_len + 1);
                char* val = malloc(val_len + 1);

                memcpy(key, header, key_len);
                memcpy(val, p + 1, val_len);
                key[key_len] = 0;
                val[val_len] = 0;

                PIE_TRACE("URL query parameter: %s", header);
                PIE_TRACE("Extracted key '%s' with value '%s'",
                          key, val);

                hmap_set(h, key, val);
        }

        return h;
}

int pie_http_get_cookie(char* restrict v,
                        struct lws* wsi,
                        const char* restrict cookie,
                        size_t len)
{
        char buf[MAX_HEADERS];
        char* t;
        char* lasts;
        int n = lws_hdr_total_length(wsi, WSI_TOKEN_HTTP_COOKIE);
        int found = -1;

        if (n == 0)
        {
                PIE_DEBUG("No cookie header found");
                return found;
        }

        lws_hdr_copy(wsi,
                     buf,
                     MAX_HEADERS,
                     WSI_TOKEN_HTTP_COOKIE);
        if (strlen(buf) == 0)
        {
                return found;
        }

        t = strtok_r(buf, ";", &lasts);
        while (t)
        {
                /* Trim away any leading spaces */
                while (*t == 0x20)
                {
                        t++;
                }

                char* p = strchr(t, '=');
                if (p == NULL)
                {
                        PIE_LOG("Found invalid cookie: '%s'", t);
                }
                else
                {
                        *p++ = 0;
                        if (strcmp(t, cookie) == 0)
                        {
                                strncpy(v, p, len);
                                found = 0;
                                break;
                        }
                }
                t = strtok_r(NULL, ";", &lasts);
        }

        return found;
}

struct pie_http_sess* pie_http_get_session(struct pie_http_sess_mgr* sess_mgr,
                                           struct lws* wsi)
{
        char buf[256];
        struct pie_http_sess* session = NULL;

        if (pie_http_get_cookie(buf, wsi, "pie-session", 256))
        {
                PIE_WARN("No pie-session cookie found");
        }
        else
        {
                session = pie_http_sess_mgr_get(sess_mgr,
                                                buf);
        }

        return session;
}

ssize_t pie_http_lws_write(struct lws* wsi,
                           unsigned char* restrict buf,
                           size_t content_len,
                           const char* restrict content_type)
{
        unsigned char resp_headers[256];
        unsigned char* hp = &resp_headers[0];
        size_t wlen;
        int bw;

        if (lws_add_http_header_status(wsi,
                                       HTTP_STATUS_OK,
                                       &hp,
                                       resp_headers + 256))
        {
                PIE_ERR("Can not write status");
                return -1;
        }

        if (lws_add_http_header_by_token(wsi,
                                         WSI_TOKEN_HTTP_CONTENT_TYPE,
                                         (unsigned char*)content_type,
                                         (int)strlen(content_type),
                                         &hp,
                                         resp_headers + 256))
        {
                PIE_ERR("Can not write content type");
                return -1;
        }

        if (lws_add_http_header_content_length(wsi,
                                               content_len,
                                               &hp,
                                               resp_headers + 256))
        {
                PIE_ERR("Can not write content len");
                return -1;
        }

        /* FIMXE: remove this in future */
        if (lws_add_http_header_by_name(wsi,
                                        (unsigned char*)"Access-Control-Allow-Origin:",
                                        (unsigned char*)"*",
                                        1,
                                        &hp,
                                        resp_headers + 256))
        {
                PIE_ERR("Can not write CORS header");
                return -1;
        }

        if (lws_finalize_http_header(wsi, &hp, resp_headers + 256))
        {
                PIE_ERR("Can not finalize headers");
                return -1;
        }

        wlen = hp - resp_headers;
        bw = lws_write(wsi, resp_headers, wlen, LWS_WRITE_HTTP_HEADERS);
        if (bw < (int)wlen)
        {
                PIE_ERR("lws_write error");
                return -1;
        }
        bw = lws_write(wsi,
                       buf,
                       content_len,
                       LWS_WRITE_HTTP);
        if (bw < (int)content_len)
        {
                PIE_ERR("lws_write error");
                return -1;
        }

        return (ssize_t)bw;
}

enum pie_http_verb pie_http_verb_get(struct lws* lws)
{
        enum pie_http_verb v = PIE_HTTP_VERB_UNKNOWN;

        if (lws_hdr_total_length(lws, WSI_TOKEN_GET_URI))
        {
                v = PIE_HTTP_VERB_GET;
        }
        else if (lws_hdr_total_length(lws, WSI_TOKEN_PUT_URI))
        {
                v = PIE_HTTP_VERB_PUT;
        }
        else if (lws_hdr_total_length(lws, WSI_TOKEN_POST_URI))
        {
                v = PIE_HTTP_VERB_POST;
        }
        else if (lws_hdr_total_length(lws, WSI_TOKEN_DELETE_URI))
        {
                v = PIE_HTTP_VERB_DELETE;
        }

        return v;
}

const char* pie_http_verb_string(enum pie_http_verb v)
{
        char* ret = "UNKNOWN";

        switch(v)
        {
        case PIE_HTTP_VERB_GET:
                ret = "GET";
                break;
        case PIE_HTTP_VERB_PUT:
                ret = "PUT";
                break;
        case PIE_HTTP_VERB_POST:
                ret = "POST";
                break;
        case PIE_HTTP_VERB_DELETE:
                ret = "DELETE";
                break;
        default:
                break;
        }

        return ret;
}

int pie_http_post_data_init(struct pie_http_post_data* p, size_t len)
{
        p->data = malloc(len);

        if (p->data == NULL)
        {
                return 1;
        }

        p->p = 0;
        p->cap = len;

        return 0;
}

int pie_http_post_data_add(struct pie_http_post_data* p,
                           const void* s,
                           size_t len)
{
        if (p->p + len > p->cap)
        {
                size_t cap = p->cap * 2;
                char* new = realloc(p->data, cap);

                if (new == NULL)
                {
                        return 1;
                }

                p->data = new;
                p->cap = cap;
        }

        memcpy(p->data + p->p, s, len);
        p->p += len;

        return 0;
}
