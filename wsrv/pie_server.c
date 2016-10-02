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

#include "pie_server.h"
#include <libwebsockets.h>

enum pie_protocols
{
        PIE_PROTO_HTTP,
        PIE_PROTO_IMG,
        PIE_PROTO_HIST,
        PIE_PROTO_CMD,
        PIE_PROTO_COUNT
};

struct pie_ctx_http
{
        lws_filefd_type fd;
        char post_data[256];
        char client_done;
};

struct pie_ctx_img
{
        int data;
};

struct pie_ctx_hist
{
        int data;
};

struct pie_ctx_cmd
{
        int data;
};

/* Global path, if multiple servers are started they MUST use the same
 * context root */
static const char* context_root;

/**
 * Return the mime type from the requested URL.
 * @param the request URL.
 * @return the mimetype (e.g image/jpeg) or NULL if unknown.
 */
const char* get_mimetype(const char*);

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
static int cb_http(struct lws *wsi, 
                   enum lws_callback_reasons reason, 
                   void *user,
                   void *in,
                   size_t len);
static int cb_img(struct lws *wsi,
                  enum lws_callback_reasons reason,
                  void *user,
                  void *in, 
                  size_t len);
static int cb_hist(struct lws *wsi,
                   enum lws_callback_reasons reason,
                   void *user,
                   void *in, 
                   size_t len);
static int cb_cmd(struct lws *wsi,
                  enum lws_callback_reasons reason,
                  void *user,
                  void *in, 
                  size_t len);

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

int start_server(struct server* srv)
{
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
                {
                        "pie-img",
                        cb_img,
                        sizeof(struct pie_ctx_img),
                        0, 
                        0,
                        NULL
                },
                {
                        "pie-hist",
                        cb_hist,
                        sizeof(struct pie_ctx_hist),
                        0, 
                        0,
                        NULL
                },
                {
                        "pie-cmd",
                        cb_cmd,
                        sizeof(struct pie_ctx_cmd),
                        0, 
                        0,
                        NULL
                },
                {NULL, NULL, 0, 0, 0, NULL}
        };
        struct lws_context_creation_info info;
        int status;

        context_root = srv->context_root;
        memset(&info, 0, sizeof(info));
        info.port = srv->port;
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

        srv->context = lws_create_context(&info);
        if (srv->context == NULL)
        {
                return 1;
        }
        srv->run = 1;

        /* start loop */
        status = 1;
        while (status >= 0 && srv->run)
        {
                /* Run state machine here */
                /* Check for stop condition or img/hist is computed */
                /* If nothing to do, return after 100ms */
                status = lws_service(srv->context, 100);
        }

        lws_context_destroy(srv->context);
        
        return 0;
}

static int cb_http(struct lws *wsi, 
                   enum lws_callback_reasons reason, 
                   void *user,
                   void *in,
                   size_t len)
{
	/* unsigned char buffer[LWS_PRE + 4096]; */
        char url[256];
        const char* mimetype;
        char headers[256];
        int n;

        switch (reason)
        {
        case LWS_CALLBACK_HTTP:
		if (len < 1)
                {
			lws_return_http_status(wsi,
                                               HTTP_STATUS_BAD_REQUEST,
                                               NULL);
			goto keepalive;
		}

                strcpy(url, context_root);

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
		if (!mimetype)
                {
			lws_return_http_status(wsi,
                                               HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE,
                                               NULL);
                        goto keepalive;
		}

                /* TODO add cookies */
                /* Serve file async */
                n = lws_serve_http_file(wsi, url, mimetype, headers, 0);
                if (n < 0 || (n > 0 && lws_http_transaction_completed(wsi)))
                {
                        return -1;
                }

                break;

	case LWS_CALLBACK_HTTP_FILE_COMPLETION:
                break;
        }
        return 0;

/* HTTP/1.1 or 2.0, default is to keepalive */
keepalive:
	if (lws_http_transaction_completed(wsi))
        {
		return -1;                
        }

	return 0;
}

static int cb_img(struct lws *wsi,
                  enum lws_callback_reasons reason,
                  void *user,
                  void *in, 
                  size_t len)
{
        int ret = -1;
        unsigned char buf[LWS_PRE + 1024];
        unsigned char* p = &buf[LWS_PRE];

        printf("HEJ %s %d\n", __func__, reason);

        switch (reason)
        {
        case LWS_CALLBACK_RECEIVE:
                printf("[cb_img]Got data: '%s'\n", in);

                break;
        }

        printf("[cb_img]Frame length: %lu\n", len);

        ret = 0;

        return ret;
}

static int cb_hist(struct lws *wsi,
                   enum lws_callback_reasons reason,
                   void *user,
                   void *in, 
                   size_t len)
{
        int ret = -1;
        unsigned char buf[LWS_PRE + 1024];
        unsigned char* p = &buf[LWS_PRE];

        printf("HEJ %s %d\n", __func__, reason);

        switch (reason)
        {
        case LWS_CALLBACK_RECEIVE:
                printf("[cb_hist]Got data: '%s'\n", in);

                break;
        }

        printf("[cb_hist]Frame length: %lu\n", len);

        ret = 0;

        return ret;
}

static int cb_cmd(struct lws *wsi,
                  enum lws_callback_reasons reason,
                  void *user,
                  void *in, 
                  size_t len)
{
        int ret = -1;
        unsigned char buf[LWS_PRE + 1024];
        unsigned char* p = &buf[LWS_PRE];
        int data[4] = {123, 234, 345, 456};
        int bw;

        printf("HEJ %s %d\n", __func__, reason);

        switch (reason)
        {
        case LWS_CALLBACK_SERVER_WRITEABLE:
                bw = lws_write(wsi, (unsigned char*)data, sizeof(data), LWS_WRITE_BINARY);
                if (bw < sizeof(data))
                {
                        printf("Error write\n");
                }
                break;
        case LWS_CALLBACK_RECEIVE:
                printf("[cb_cmd]Got data: '%s'\n", in);
                lws_callback_on_writable(wsi);
                break;
        }

        printf("[cb_cmd]Frame length: %lu\n", len);

        ret = 0;

        return ret;
}

const char* get_mimetype(const char *path)
{
	int n = strlen(path);

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

	return NULL;
}
