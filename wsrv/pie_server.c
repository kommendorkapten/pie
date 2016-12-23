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
#include "pie_session.h"
#include "pie_cmd.h"
#include "../pie_log.h"
#include "../pie_types.h"
#include "../msg/pie_msg.h"
#include "../lib/chan.h"
#include "../lib/timing.h"
#include <libwebsockets.h>
#include <string.h>

#define MAX_HEADERS 4096

#ifdef __sun
# include <note.h>
#else
# define NOTE(X)
#endif

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
        char post_data[256];
        char token[PIE_SESS_TOKEN_LEN];
        lws_filefd_type fd;
};

struct pie_ctx_img
{
        char token[PIE_SESS_TOKEN_LEN];
};

struct pie_ctx_hist
{
        char token[PIE_SESS_TOKEN_LEN];
};

struct pie_ctx_cmd
{
        char token[PIE_SESS_TOKEN_LEN];
};

/* Global root to asset directory */
static const char* context_root;
/* Only one server can be executed at a time */
static struct pie_server* server;
static struct pie_sess_mgr* sess_mgr;

/**
 * Return the mime type from the requested URL.
 * @param the request URL.
 * @return the mimetype (e.g image/jpeg) or NULL if unknown.
 */
static const char* get_mimetype(const char*);

/**
 * Extract a session from the current request.
 * A session is defined by the cookie pie-session.
 * @param the request
 * @return pointer to the session, or NULL if no session is found.
 */
static struct pie_sess* get_session(struct lws*);

/**
 * Init the session with current server related data.
 * @param the session to init.
 * @return void.
 */
static void srv_init_session(struct pie_sess*);

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
static int cb_img(struct lws* wsi,
                  enum lws_callback_reasons reason,
                  void* user,
                  void* in, 
                  size_t len);
static int cb_hist(struct lws* wsi,
                   enum lws_callback_reasons reason,
                   void* user,
                   void* in, 
                   size_t len);
static int cb_cmd(struct lws* wsi,
                  enum lws_callback_reasons reason,
                  void* user,
                  void* in, 
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

int start_server(struct pie_server* srv)
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

        sess_mgr = pie_sess_mgr_create();

        context_root = srv->context_root;
        server = srv;

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
                /* Receiver is responsible to free data */
                struct chan_msg msg;
                int n;

                n = chan_read(srv->response, &msg, 0);
                if (n == 0)
                {
                        struct pie_msg* resp = (struct pie_msg*)msg.data;
                        struct pie_sess* session;

                        PIE_DEBUG("[%s] received message %d (RTT %ldusec).",
                                  resp->token,
                                  (int)resp->type,
                                  timing_dur_usec(&resp->t));

                        if (msg.len != sizeof(struct pie_msg))
                        {
                                PIE_WARN("Invalid message received.");
                                goto msg_done;
                        }
                        
                        session = pie_sess_mgr_get(sess_mgr, resp->token);
                        if (session == NULL)
                        {
                                PIE_ERR("[%s] sesion not found.",
                                       resp->token);
                                goto msg_done;
                        }
                        switch (resp->type)
                        {
                        case PIE_MSG_LOAD_DONE:
                                /* On load done, a new image workspace 
                                   is created, store it in the session. */
                                session->img = resp->img;
                        NOTE(FALLTHRU)
                        case PIE_MSG_RENDER_DONE:
                                /* Update session with tx ready */
                                session->tx_ready = (PIE_TX_IMG | PIE_TX_HIST);
                                lws_callback_on_writable_all_protocol(srv->context, 
                                                                      &protocols[PIE_PROTO_IMG]);
                                lws_callback_on_writable_all_protocol(srv->context,
                                                                      &protocols[PIE_PROTO_HIST]);
                                break;
                        default:
                                PIE_WARN("[%s] invalid message: %d",
                                         resp->token,
                                         (int)resp->type);
                        }

                msg_done:
                        pie_msg_free(msg.data);
                }
                else if (n != EAGAIN)
                {
                        PIE_ERR("Error reading channel %d.", n);
                        srv->run = 0;
                }

                /* Check for stop condition or img/hist is computed */
                /* If nothing to do, return after Xms */
                status = lws_service(srv->context, 5);
                /* Reap sessions with inactivity for one hour */
                pie_sess_mgr_reap(sess_mgr, 60 * 60);
        }

        PIE_LOG("Shutdown server.");
        lws_context_destroy(srv->context);
        pie_sess_mgr_destroy(sess_mgr);
        
        return 0;
}

static int cb_http(struct lws* wsi, 
                   enum lws_callback_reasons reason, 
                   void* user,
                   void* in,
                   size_t len)
{
        char url[256];
        unsigned char headers[256];
        struct pie_sess* session;
        const char* mimetype;
        struct pie_ctx_http* ctx = (struct pie_ctx_http*)user;
        int hn = 0;
        int n;

        switch (reason)
        {
        case LWS_CALLBACK_HTTP:
                /* Look for existing session */
                session = get_session(wsi);
                if (session == NULL)
                {
                        /* Create a new */
                        char cookie[128];
                        unsigned char* p = &headers[0];
                        session = malloc(sizeof(struct pie_sess));
                        pie_sess_init(session);
                        srv_init_session(session);
                        
                        hn = snprintf(cookie, 
                                      128,
                                      "pie-session=%s;Max Age=3600",
                                      session->token);
                        if (lws_add_http_header_by_name(wsi,
                                                        (unsigned char*)"set-cookie:",
                                                        (unsigned char*)cookie,
                                                        hn,
                                                        &p,
                                                        headers + 256))
                        {
                                /* Can't set header */
                                return -1;
                        }

                        PIE_DEBUG("[%s] Init session",
                                  session->token);

                        pie_sess_mgr_put(sess_mgr, session);
                        /* 
                         * This is usually safe. P should never be incremented
                         * more than the size of headers.
                         * P can never be decremented.
                         * But it's not beautiful, but needed to silence lint.
                         */
                        hn = (int)((unsigned long)p - (unsigned long)&headers[0]);
                        strncpy(ctx->token, 
                                session->token, 
                                PIE_SESS_TOKEN_LEN);
                }

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
                PIE_TRACE("[%s] GET %s",
                          session->token,
                          url);
                
		if (!mimetype)
                {
			lws_return_http_status(wsi,
                                               HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE,
                                               NULL);
                        goto keepalive;
		}

                /* Serve file async */
                n = lws_serve_http_file(wsi,
                                        url,
                                        mimetype,
                                        (char*)headers,
                                        hn);
                if (n < 0 || (n > 0 && lws_http_transaction_completed(wsi)))
                {
                        return -1;
                }

                break;

	case LWS_CALLBACK_HTTP_FILE_COMPLETION:
                break;
        default:
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

static int cb_img(struct lws* wsi,
                  enum lws_callback_reasons reason,
                  void* user,
                  void* in, 
                  size_t len)
{
        struct pie_sess* session = NULL;
        struct pie_ctx_img* ctx = (struct pie_ctx_img*)user;
        int ret = 0;

        (void)len;

        if (user && reason != LWS_CALLBACK_ESTABLISHED)
        {
                session = pie_sess_mgr_get(sess_mgr, ctx->token);
        }

        switch (reason)
        {
        case LWS_CALLBACK_ESTABLISHED:
                /* Copy the session token, it is not available later on */
                if ((session = get_session(wsi)))
                {
                        strcpy(ctx->token, session->token);
                }
                else
                {
                        PIE_WARN("No session found");
                        ret = -1;
                }
                break;
        case LWS_CALLBACK_SERVER_WRITEABLE:
                if (!session)
                {
                        PIE_WARN("No session found");
                        ret = -1;
                        break;
                }
                PIE_DEBUG("[%s] tx_ready: 0x%x",
                          session->token,
                          session->tx_ready);

                if (session->tx_ready & PIE_TX_IMG)
                {
                        int bw;

                        bw = lws_write(wsi, 
                                       session->img->proxy_out_rgba,
                                       session->img->proxy_out_len, 
                                       LWS_WRITE_BINARY);
                        if (bw < session->img->proxy_out_len)
                        {
                                PIE_ERR("[%s] ERROR write %d of %d",
                                        session->token,
                                        bw,
                                        session->img->proxy_out_len);
                                ret = -1;
                        }
                        session->tx_ready = (unsigned char)(session->tx_ready ^ PIE_TX_IMG);
                }
                break;
        case LWS_CALLBACK_RECEIVE:
                PIE_LOG("[%s] Got data: '%s'", 
                        session->token,
                        (char*)in);

                break;
        default:
                break;
        }

        return ret;
}

static int cb_hist(struct lws* wsi,
                   enum lws_callback_reasons reason,
                   void* user,
                   void* in, 
                   size_t len)
{
        struct pie_sess* session = NULL;
        struct pie_ctx_hist* ctx = (struct pie_ctx_hist*)user;
        int ret = 0;

        (void)len;

        if (user && reason != LWS_CALLBACK_ESTABLISHED)
        {
                session = pie_sess_mgr_get(sess_mgr, ctx->token);
        }

        switch (reason)
        {
        case LWS_CALLBACK_ESTABLISHED:
                /* Copy the session token, it is not available later on */
                if ((session = get_session(wsi)))
                {
                        strcpy(ctx->token, session->token);

                }
                else
                {
                        PIE_WARN("No session found");
                        ret = -1;
                }
                break;
        case LWS_CALLBACK_SERVER_WRITEABLE:
                if (!session)
                {
                        PIE_WARN("No session found");
                        ret = -1;
                        break;
                }

                PIE_DEBUG("[%s] tx_ready: 0x%x",
                          session->token,
                          session->tx_ready);

                if (session->tx_ready & PIE_TX_HIST)
                {
                        int bw;

                        bw = lws_write(wsi,
                                       session->img->hist_json,
                                       session->img->hist_json_len,
                                       LWS_WRITE_TEXT);
                        if (bw < session->img->hist_json_len)
                        {
                                PIE_ERR("[%s] ERROR write %d of %d",
                                        session->token,
                                        bw,
                                        session->img->hist_json_len);
                                ret = -1;
                        }
                        session->tx_ready = (unsigned char)(session->tx_ready ^ PIE_TX_HIST);
                }
                break;
        case LWS_CALLBACK_RECEIVE:
                PIE_LOG("[%s]Got data: '%s'",
                        session->token,
                        (char*)in);

                break;
        default:
                break;
        }

        return ret;
}

static int cb_cmd(struct lws* wsi,
                  enum lws_callback_reasons reason,
                  void* user,
                  void* in, 
                  size_t len)
{
        struct chan_msg envelope;
        struct pie_sess* session = NULL;
        struct pie_msg* msg;
        struct pie_ctx_cmd* ctx = (struct pie_ctx_cmd*)user;
        int ret = 0;

        if (user && reason != LWS_CALLBACK_ESTABLISHED)
        {
                session = pie_sess_mgr_get(sess_mgr, ctx->token);
        }

        switch (reason)
        {
        case LWS_CALLBACK_ESTABLISHED:
                /* Copy the session token, it is not available later on */
                if ((session = get_session(wsi)))
                {
                        strcpy(ctx->token, session->token);
                }
                else
                {
                        PIE_WARN("No session found");
                        return -1;
                }
                break;
        case LWS_CALLBACK_RECEIVE:
                if (!session)
                {
                        PIE_WARN("No session found");
                        return -1;
                }
                msg = pie_msg_alloc();
                strncpy(msg->token, session->token, PIE_MSG_TOKEN_LEN);
                if (parse_cmd_msg(msg, (char*)in, len))
                {
                        PIE_WARN("failed to parse message: '%s'",
                                 (char*)in);
                }
                else
                {
                        if (msg->type != PIE_MSG_LOAD && session->img == NULL)
                        {
                                PIE_LOG("[%s] No image loaded",
                                       session->token);
                                goto done;
                        }
                        /* Copy image from session to message */
                        msg->img = session->img;
                        envelope.data = msg;
                        envelope.len = sizeof(struct pie_msg);
                        timing_start(&msg->t);
                        
                        if (chan_write(session->command, &envelope))
                        {
                                PIE_ERR("failed to write to chan");
                        }
                }

#if 0
                msg->type = PIE_MSG_SET_CONTRAST;
                msg->i1 = 50;

                if (chan_write(session->command, &envelope))
                {
                        PIE_ERR("failed to write to chan\n");
                }

                /* HACK */
                msg = pie_msg_alloc();
                msg->type = PIE_MSG_LOAD;
                strncpy(msg->token, session->token, PIE_MSG_TOKEN_LEN);
                envelope.data = msg;
                envelope.len = sizeof(struct pie_msg);
#endif

                break;
        default:
                break;
        }

done:
        return ret;
}

static const char* get_mimetype(const char *path)
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

static struct pie_sess* get_session(struct lws* wsi)
{
        char headers[MAX_HEADERS];
        struct pie_sess* session = NULL;
        int n = lws_hdr_total_length(wsi, WSI_TOKEN_HTTP_COOKIE);
        
        if (n == 0)
        {
                PIE_DEBUG("No cookie header found");
                return NULL;
        }
        
        lws_hdr_copy(wsi,
                     headers,
                     MAX_HEADERS,
                     WSI_TOKEN_HTTP_COOKIE);
        if (strlen(headers))
        {
                char* t = strchr(headers, '=');

                if (t == NULL)
                {
                        PIE_DEBUG("Invalid cookie: '%s'", t);
                        return NULL;
                }
                t++;
                session = pie_sess_mgr_get(sess_mgr,
                                           t);
        }
        return session;
}

static void srv_init_session(struct pie_sess* s)
{
        s->command = server->command;
        s->response = server->response;
        s->img = NULL;
        s->tx_ready = 0;
}
