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

#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <libwebsockets.h>
#include "pie_coll_handler.h"
#include "../cfg/pie_cfg.h"
#include "../pie_log.h"
#include "../lib/hmap.h"
#include "../http/pie_util.h"

#define ROUTE_COLLECTION "/collection/"
#define RESP_LEN (1 << 12) /* 4k */

struct config
{
        const char* lib_path;
        const char* context_root;
        const char* thumbs_root;
        struct lws_context* context;
        int port;
};

/* struct config2 */
/* { */
        
/* }; */

enum pie_protocols
{
        PIE_PROTO_HTTP,
        PIE_PROTO_COUNT
};

struct pie_ctx_http
{
        char post_data[256];
        lws_filefd_type fd;
};

static void sig_h(int);

/**
 * Callback methods.
 * @param The web-sockets instance.
 * @param The reason fro the callback. 
 *        See libwebsockets/enum lws_callback_reasons for alternatives.
 * @param The per session context data.
 * @param Incoming data for this request. (Path for HTTP, data for WS etc).
 * @param Length of data for request.
 * @return 0 on sucess, negative otherwise.
 */
static int cb_http(struct lws* wsi, 
                   enum lws_callback_reasons reason, 
                   void* user,
                   void* in,
                   size_t len);

volatile int run;
static struct config cfg;
static const struct lws_extension exts[] = {
        {
                "permessage-deflate",
                lws_extension_callback_pm_deflate,
                "permessage-deflate"
        },
        {
                "deflate-frame",
                lws_extension_callback_pm_deflate,
                "deflate_frame"
        },
        {NULL, NULL, NULL}
};

int main(void)
{
        struct sigaction sa;
        struct lws_protocols protocols[] = {
                /* HTTP must be first */
                {
                        "http-only", /* name */
                        cb_http,     /* callback */
                        sizeof(struct pie_ctx_http), /* ctx size */
                        0,           /* max frame size / rx buffer */
                        0,           /* id, not used */
                        NULL         /* void* user data, let lws allocate */
                },
                {NULL, NULL, 0, 0, 0, NULL}
        };
        struct lws_context_creation_info info;
        int status = 1;

        if (pie_cfg_load(PIE_CFG_PATH))
        {
                PIE_ERR("Failed to read conf");
                return 1;
        }
        
        cfg.lib_path= "test-images";
        cfg.context_root = "assets";
        cfg.thumbs_root = "thumbs";
        cfg.port = 8081;

        sa.sa_handler = &sig_h;
        sa.sa_flags = 0;
        sigfillset(&sa.sa_mask);
        if (sigaction(SIGINT, &sa, NULL))
        {
                perror("i_handler::sigaction");
                return 1;
        }

        /* Prepare web server */
        memset(&info, 0, sizeof(info));
        info.port = cfg.port;
        info.iface = NULL; /* all ifaces */
        info.ssl_cert_filepath = NULL;
        info.ssl_private_key_filepath = NULL;
        info.protocols = protocols;
        info.gid = -1;
        info.uid = -1;
        info.max_http_header_pool = 16;
        info.options = LWS_SERVER_OPTION_VALIDATE_UTF8;
        info.extensions = exts;
        info.timeout_secs = 5;
        info.ssl_cipher_list = "ECDHE-ECDSA-AES256-GCM-SHA384:"
                               "ECDHE-RSA-AES256-GCM-SHA384:"
                               "DHE-RSA-AES256-GCM-SHA384:"
                               "ECDHE-RSA-AES256-SHA384:"
                               "HIGH:!aNULL:!eNULL:!EXPORT:"
                               "!DES:!MD5:!PSK:!RC4:!HMAC_SHA1:"
                               "!SHA1:!DHE-RSA-AES128-GCM-SHA256:"
                               "!DHE-RSA-AES128-SHA256:"
                               "!AES128-GCM-SHA256:"
                               "!AES128-SHA256:"
                               "!DHE-RSA-AES256-SHA256:"
                               "!AES256-GCM-SHA384:"
                               "!AES256-SHA256";

        cfg.context = lws_create_context(&info);
        if (cfg.context == NULL)
        {
                PIE_ERR("Could not create context");
                return 1;
        }

        /* Serve forever */
        run = 1;
        status = 1;
        while (status >= 0 && run)
        {
                status = lws_service(cfg.context, 50);
        }

        PIE_LOG("Shutdown server.");
        lws_context_destroy(cfg.context);        
        
        return 0;
}

static void sig_h(int signum)
{
        if (signum == SIGINT)
        {
                run = 0;
        }
}

static int cb_http(struct lws* wsi, 
                   enum lws_callback_reasons reason, 
                   void* user,
                   void* in,
                   size_t len)
{
        char url[256];
        unsigned char resp_headers[256];
        struct hmap* req_headers = NULL;
        const char* mimetype;
        const char* p;
        /*struct pie_ctx_http* ctx = (struct pie_ctx_http*)user;*/
        int hn = 0;
        int n;
        int ret;

        (void)user;
        
        /* Set to true if callback should attempt to keep the connection
           open. */
        int try_keepalive = 0;

        switch (reason)
        {
        case LWS_CALLBACK_HTTP:
                try_keepalive = 1;
                req_headers = get_request_headers(wsi);

		if (len < 1)
                {
			lws_return_http_status(wsi,
                                               HTTP_STATUS_BAD_REQUEST,
                                               NULL);
                        PIE_DEBUG("Bad request, inpupt len %lu", len);
			goto keepalive;
		}

                if (strcmp(in, ROUTE_COLLECTION) == 0)
                {
                        char resp[RESP_LEN];
                        char* content_type = "application/json; charset=UTF-8";
                        size_t bw;

                        bw = pie_coll_h_collections(resp,
                                                    RESP_LEN,
                                                    pie_cfg_get_db());
                        if (pie_http_lws_write(wsi,
                                               (unsigned char*) resp,
                                               bw,
                                               content_type) < 0)
                        {
                                PIE_ERR("FAILED TO WRITE");
                                goto bailout;
                        }
                        goto keepalive;
                }

                /* route is /collection/\d+$ */
                p = strstr(in, ROUTE_COLLECTION);
                if (p == in)
                {
                        char resp[RESP_LEN];
                        char* content_type = "application/json; charset=UTF-8";
                        char* id;
                        char* p;
                        size_t bw;
                        pie_id coll_id;

                        PIE_LOG("GET '%s'", (char*)in);
                        id = strchr(((char*)in) + 1, '/');
                        if (id == NULL)
                        {
                                PIE_ERR("Slash dissapeared from requested URL: '%s'", (char*)in);
                                goto bailout;
                        }
                        /* Advance pointer to frist char after '/' */
                        id++;
                        p = id;

                        while (*p)
                        {
                                if (!isdigit(*p++))
                                {
                                        lws_return_http_status(wsi,
                                                               HTTP_STATUS_BAD_REQUEST,
                                                               NULL);
                                        goto bailout;
                                }
                        }
                        /* The error check here is actualy not needed,
                           but better to be safe than sorry. */
                        coll_id = strtol(id, &p, 10);
                        if (id == p)
                        {
                                lws_return_http_status(wsi,
                                                       HTTP_STATUS_BAD_REQUEST,
                                                       NULL);
                                goto bailout;
                        }

                        bw = pie_coll_h_collection(resp,
                                                   RESP_LEN,
                                                   pie_cfg_get_db(),
                                                   coll_id);
                        if (pie_http_lws_write(wsi,
                                               (unsigned char*)resp,
                                               bw,
                                               content_type) < 0)
                        {
                                PIE_ERR("Failed to write");
                                goto bailout;
                        }

                        lws_callback_on_writable(wsi);
                }
                else
                {
                        /* Serve static file */
                        strcpy(url, cfg.context_root);

                        /* Get the URL */
                        if (strcmp(in, "/"))
                        {
                                /* File provided */
                                strncat(url, in, sizeof(url) - strlen(url) - 1);
                        }
                        else 
                        {
                                /* Get index.html */
                                strcat(url, "/index.html");
                        }
                        url[sizeof(url) - 1] = 0;
                
                        mimetype = get_mimetype(url);
                        PIE_LOG("GET %s", url);
                
                        if (!mimetype)
                        {
                                lws_return_http_status(wsi,
                                                       HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE,
                                                       NULL);
                                PIE_DEBUG("Bad mime type: %s", mimetype);
                                goto keepalive;
                        }

                        /* Serve file async */
                        n = lws_serve_http_file(wsi,
                                                url,
                                                mimetype,
                                                (char*)resp_headers,
                                                hn);
                        if (n == 0)
                        {
                                /* File is beeing served, but we can not 
                                   check for transaction completion yet */
                                try_keepalive = 0;
                        }
                        if (n < 0)
                        {
                                PIE_WARN("Fail to send file '%s'", url);
                                goto bailout;
                        }
                }
                

                break;
        case LWS_CALLBACK_CLOSED_HTTP:
                PIE_LOG("Timeout, closing.");
                goto bailout;
        case LWS_CALLBACK_HTTP_WRITEABLE:
                try_keepalive = 1;
                PIE_LOG("Book");
                lws_callback_on_writable(wsi);
                break;
        default:
                break;
        }
        
/* HTTP/1.1 or 2.0, default is to keepalive */
keepalive:
        ret = 0;

        if (try_keepalive)
        {
                PIE_DEBUG("Check for completion");
                if (lws_http_transaction_completed(wsi))
                {
                        PIE_WARN("Failed to keep connection open");
                        goto bailout;
                }
        }        
        goto cleanup;
bailout:
        ret = -1;
cleanup:
        if (req_headers)
        {
                size_t h_size;
                struct hmap_entry* it = hmap_iter(req_headers, &h_size);

                for (size_t i = 0; i < h_size; i++)
                {
                        free(it[i].key);
                        free(it[i].data);
                }
                
                free(it);
                hmap_destroy(req_headers);
        }
        
	return ret;
}
