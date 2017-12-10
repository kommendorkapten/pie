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
#include <regex.h>
#include <libwebsockets.h>
#include "pie_coll_handler.h"
#include "../cfg/pie_cfg.h"
#include "../pie_log.h"
#include "../lib/hmap.h"
#include "../http/pie_util.h"
#include "../dm/pie_host.h"
#include "../dm/pie_storage.h"

#define RESP_LEN (1 << 20) /* 1M */
#define MAX_URL 256
#define MAX_ROUTES 16

/**
 * Callback from HTTP router.
 * @param response struct.
 * @param request url.
 * @param HTTP method
 * @param request body.
 * @param database handle.
 * @return 0 on success.
 */
typedef int (colld_handler)(struct pie_coll_h_resp*,
                            const char*,
                            enum pie_http_verb,
                            struct pie_http_post_data*,
                            sqlite3*);

struct config
{
        struct pie_stg_mnt* thumb_stg;
        struct pie_stg_mnt* proxy_stg;
        struct pie_stg_mnt* online_stg;
        struct pie_stg_mnt_arr* storages;
        struct pie_host* host;
        const char* context_root;
        struct lws_context* context;
        int port;
};

enum pie_protocols
{
        PIE_PROTO_HTTP,
        PIE_PROTO_COUNT
};

struct pie_ctx_http
{
        char url[MAX_URL];
        struct pie_http_post_data post_data;
        enum pie_http_verb verb;
};

struct colld_route
{
        regex_t* r;
        colld_handler* h;
};

static void sig_h(int);

/**
 * Callback methods.
 * @param The web-sockets instance.
 * @param The reason for the callback.
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

static void install_handlers(void);
static colld_handler* route_request(const char* url);

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
static struct colld_route handlers[MAX_ROUTES];
char gresp[RESP_LEN];

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

        install_handlers();

        if (pie_cfg_load(PIE_CFG_PATH))
        {
                PIE_ERR("Failed to read conf");
                return 1;
        }

        cfg.host = pie_cfg_get_host(-1);
        if (cfg.host == NULL)
        {
                PIE_ERR("Could not load current host");
                return 1;
        }
        cfg.storages = pie_cfg_get_hoststg(cfg.host->hst_id);
        if (cfg.storages == NULL)
        {
                PIE_ERR("Could not resolve mount point for storage");
                return 1;
        }
        for (int i = 0; i < cfg.storages->len; i++)
        {
                if (cfg.storages->arr[i])
                {
                        PIE_LOG("Storage %d at %s",
                                cfg.storages->arr[i]->stg.stg_id,
                                cfg.storages->arr[i]->mnt_path);

                        switch(cfg.storages->arr[i]->stg.stg_type)
                        {
                        case PIE_STG_ONLINE:
                                cfg.online_stg = cfg.storages->arr[i];
                                break;
                        case PIE_STG_THUMB:
                                cfg.thumb_stg = cfg.storages->arr[i];
                                break;
                        case PIE_STG_PROXY:
                                cfg.proxy_stg = cfg.storages->arr[i];
                                break;
                        }
                }
        }
        cfg.port = 8081;
        cfg.context_root = "assets";

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

        pie_host_free(cfg.host);
        pie_cfg_free_hoststg(cfg.storages);

        for (int h = 0; handlers[h].r != NULL; h++)
        {
                regfree(handlers[h].r);
                h++;
        }

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
        unsigned char resp_headers[256];
        char file_url[256];
        struct pie_coll_h_resp resp;
        struct hmap* query_params = NULL;
        const char* req_url = in;
        const char* p;
        struct pie_ctx_http* ctx = user;
        const char* mimetype;
        colld_handler* handler;
        /* current header position */
        unsigned char* hp = resp_headers;
        /* total header length */
        int hn = 0;
        int ret = 0;
        /* Set to true if callback should attempt to keep the connection
           open. */
        int try_keepalive = 0;
        enum pie_http_verb verb = PIE_HTTP_VERB_UNKNOWN;

        resp.wbuf = &gresp[0];
        resp.wbuf_len = RESP_LEN;
        resp.http_sc = HTTP_STATUS_BAD_REQUEST;
        resp.content_type = NULL;
        resp.content_len = 0;

        file_url[0] = '\0';

        switch (reason)
        {
        case LWS_CALLBACK_HTTP:
                verb = pie_http_verb_get(wsi);
                try_keepalive = 1;
                query_params = pie_http_req_params(wsi);

                /* Deal with PUT/POST */
                /* First set up cb to slurp the data. When all data is slurped
                   reslove the url and route accordingly in BODY_COMPLETION
                   phase. Not efficient but easier to implement. */
                if (verb == PIE_HTTP_VERB_PUT || verb == PIE_HTTP_VERB_POST)
                {
                        if (pie_http_post_data_init(&ctx->post_data,
                                                    2048))
                        {
                                PIE_ERR("Could not init post data");
                                resp.http_sc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
                                goto bailout;
                        }
                        if ((len + 1) > MAX_URL)
                        {
                                PIE_ERR("To large URL");
                                resp.http_sc = HTTP_STATUS_REQ_URI_TOO_LONG;
                                goto bailout;
                        }
                        ctx->verb = verb;
                        memcpy(ctx->url, in, len);
                        ctx->url[len] = '\0';

                        ret = 0;
                        goto cleanup;
                }

                if (len < 1)
                {
                        lws_return_http_status(wsi,
                                               HTTP_STATUS_BAD_REQUEST,
                                               NULL);
                        PIE_DEBUG("Bad request, inpupt len %lu", len);
                        goto keepalive;
                }

                /* Route request */
                handler = route_request(req_url);
                if (handler)
                {
                        ret = handler(&resp,
                                      req_url,
                                      verb,
                                      &ctx->post_data,
                                      pie_cfg_get_db());
                        goto writebody;
                }

                p = strstr(req_url, "/thumb/");
                if (p == req_url)
                {
                        snprintf(file_url, sizeof(file_url),
                                 "%s%s",
                                 cfg.thumb_stg->mnt_path,
                                 req_url + 6);
                }

                p = strstr(req_url, "/proxy/");
                if (p == req_url)
                {
                        snprintf(file_url, sizeof(file_url),
                                 "%s%s",
                                 cfg.proxy_stg->mnt_path,
                                 req_url + 6);

                        /* Add CORS headers. Proxy image is used by editd*/
                        if (lws_add_http_header_by_name(wsi,
                                                        (unsigned char*)"Access-Control-Allow-Origin:",
                                                        (unsigned char*)"*",
                                                        1,
                                                        &hp,
                                                        resp_headers + 256))
                        {
                                PIE_ERR("Can not write CORS header");
                        }
                }

                /* Catch all static file */
                if (file_url[0] == '\0')
                {
                        if (strcmp(req_url, "/") == 0)
                        {
                                req_url = "/index.html";
                        }

                        snprintf(file_url, sizeof(file_url),
                                 "%s%s",
                                 cfg.context_root,
                                 req_url);
                }

                file_url[sizeof(file_url) - 1] = 0;
                PIE_LOG("Serve static file '%s'", file_url);

                mimetype = get_mimetype(file_url);
                if (!mimetype)
                {
                        lws_return_http_status(wsi,
                                               HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE,
                                               NULL);
                        PIE_DEBUG("Bad mime type: %s", mimetype);
                        goto keepalive;
                }

                hn = (int)(hp - resp_headers);
                if (hn > 0)
                {
                        /* Serve file async */
                        if (lws_finalize_http_header(wsi, &hp, resp_headers + 256))
                        {
                                PIE_ERR("Can not finalize headers");
                        }
                }
                int n = lws_serve_http_file(wsi,
                                            file_url,
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
                        PIE_WARN("Fail to send file '%s'", file_url);
                }

                goto keepalive;
        case LWS_CALLBACK_HTTP_BODY:
                PIE_TRACE("BODY [%p] '%s' %lu", user, req_url, len);
                if (pie_http_post_data_add(&ctx->post_data, in, len))
                {
                        PIE_ERR("Failed to add post data");
                        resp.http_sc = 500;
                        goto bailout;
                }

                ret = 0;
                goto cleanup;
        case LWS_CALLBACK_HTTP_BODY_COMPLETION:
                PIE_TRACE("BODY_COMPLETION %s %s [%p] %lu bytes",
                          pie_http_verb_string(ctx->verb),
                          ctx->url,
                          user,
                          ctx->post_data.p);

                resp.http_sc = HTTP_STATUS_METHOD_NOT_ALLOWED;

                /* Route request */
                handler = route_request(ctx->url);
                if (handler)
                {
                        ret = handler(&resp,
                                      ctx->url,
                                      ctx->verb,
                                      &ctx->post_data,
                                      pie_cfg_get_db());
                }

                if (ctx->post_data.data)
                {
                        free(ctx->post_data.data);
                        ctx->post_data.data = NULL;
                }

                /* Save state to not mess up logging in writebody */
                verb = ctx->verb;
                req_url = ctx->url;

                try_keepalive = 1;
                goto writebody;
        case LWS_CALLBACK_CLOSED_HTTP:
                PIE_LOG("Timeout, closing.");
                goto bailout;
        case LWS_CALLBACK_HTTP_WRITEABLE:
                try_keepalive = 1;
                lws_callback_on_writable(wsi);
                goto keepalive;
        default:
                goto keepalive;
        }

writebody:
        PIE_DEBUG("%s [%p] '%s' %d %lu",
                  pie_http_verb_string(verb),
                  user,
                  req_url,
                  resp.http_sc,
                  resp.content_len);
        if (ret || resp.http_sc > 399)
        {
                goto bailout;
        }

        if (pie_http_lws_write(wsi,
                               (unsigned char*)resp.wbuf,
                               resp.content_len,
                               resp.content_type) < 0)
        {
                PIE_ERR("Failed to write");
                goto bailout;
        }

/* HTTP/1.1 or 2.0, default is to keepalive */
keepalive:
        if (try_keepalive)
        {
                PIE_TRACE("Check for completion");
                if (lws_http_transaction_completed(wsi))
                {
                        PIE_WARN("Failed to keep connection open");
                        goto bailout;
                }
        }
        goto cleanup;

bailout:
        if (resp.http_sc > 399)
        {
                lws_return_http_status(wsi,
                                       resp.http_sc,
                                       NULL);
        }
        else
        {
                lws_return_http_status(wsi,
                                       HTTP_STATUS_INTERNAL_SERVER_ERROR,
                                       NULL);
        }
        ret = -1;

cleanup:
        if (query_params)
        {
                size_t h_size;
                struct hmap_entry* it = hmap_iter(query_params, &h_size);

                for (size_t i = 0; i < h_size; i++)
                {
                        free(it[i].key);
                        free(it[i].data);
                }

                free(it);
                hmap_destroy(query_params);
        }

	return ret;
}

static void install_handlers(void)
{
        int h = 0;

        /* Collection - modify */
        handlers[h].r = malloc(sizeof(regex_t));
        handlers[h].h = &pie_coll_h_coll_asset;
        if (regcomp(handlers[h].r, "^/collection/[0-9]+/asset/[0-9]+", REG_EXTENDED))
        {
                abort();
        }
        h++;

        /* Collection - single */
        handlers[h].r = malloc(sizeof(regex_t));
        handlers[h].h = &pie_coll_h_coll;
        if (regcomp(handlers[h].r, "^/collection/[0-9]+", REG_EXTENDED))
        {
                abort();
        }
        h++;

        /* Collection - all */
        handlers[h].r = malloc(sizeof(regex_t));
        handlers[h].h = &pie_coll_h_colls;
        if (regcomp(handlers[h].r, "^/collection/$", REG_EXTENDED))
        {
                abort();
        }
        h++;

        /* Exif */
        handlers[h].r = malloc(sizeof(regex_t));
        handlers[h].h = &pie_coll_h_exif;
        if (regcomp(handlers[h].r, "^/exif/[0-9]+", REG_EXTENDED))
        {
                abort();
        }
        h++;

        /* MOB */
        handlers[h].r = malloc(sizeof(regex_t));
        handlers[h].h = &pie_coll_h_mob;
        if (regcomp(handlers[h].r, "^/mob/[0-9]+", REG_EXTENDED))
        {
                abort();
        }
        h++;

        /* Development params */
        handlers[h].r = malloc(sizeof(regex_t));
        handlers[h].h = &pie_coll_h_devp;
        if (regcomp(handlers[h].r, "^/devparams/[0-9]+", REG_EXTENDED))
        {
                abort();
        }
        h++;

        /* Add termination */
        if (h < MAX_ROUTES - 1)
        {
                handlers[h].r = NULL;
                handlers[h].h = NULL;
        }
        if (h >= MAX_ROUTES)
        {
                PIE_ERR("To many routes configured");
                abort();
        }
}

static colld_handler* route_request(const char* url)
{
        colld_handler* handler = NULL;

        for (int h = 0; h < MAX_ROUTES; h++)
        {
                int m;

                if (handlers[h].r == NULL)
                {
                        break;
                }

                m = regexec(handlers[h].r, url, 0, NULL, 0);
                switch(m)
                {
                case 0:
                        handler = handlers[h].h;
                        goto done;
                case REG_NOMATCH:
                        break;
                case REG_ENOSYS:
                        PIE_ERR("REG_ENOSYS");
                        abort();
                default:
                        PIE_LOG("WTF: %d", m);
                }
        }
done:

        return handler;
}
