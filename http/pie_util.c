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

#include "pie_util.h"
#include "pie_session.h"
#include "../pie_log.h"
#include "../lib/hmap.h"
#include <libwebsockets.h>
#include <string.h>

const char* get_mimetype(const char *path)
{
	size_t n = strlen(path);

	if (n < 5)
        {
		return NULL;                
        }

	if (strcmp(&path[n - 4], ".ico") == 0)
        {
		return "image/x-icon";
        }
	if (strcmp(&path[n - 4], ".png") == 0)
        {
		return "image/png";
        }
	if (strcmp(&path[n - 5], ".html") == 0) 
        {
		return "text/html";
        }
	if (strcmp(&path[n - 4], ".css") == 0)
        {
		return "text/css";
        }
	if (strcmp(&path[n - 4], ".jpg") == 0)
        {
		return "text/jpeg";
        }
	if (strcmp(&path[n - 3], ".js") == 0)
        {
		return "text/javascript";
        }

	return NULL;
}

struct hmap* get_request_headers(struct lws* wsi)
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

int get_lws_cookie(char* restrict v,
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

struct pie_sess* get_session(struct pie_sess_mgr* sess_mgr, struct lws* wsi)
{
        char buf[256];
        struct pie_sess* session = NULL;

        if (get_lws_cookie(buf, wsi, "pie-session", 256))
        {
                PIE_WARN("No pie-session cookie found");
        }
        else
        {
                session = pie_sess_mgr_get(sess_mgr,
                                           buf);
        }

        return session;
}
