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
#include "../pie_types.h"
#include "../msg/pie_msg.h"
#include "../lib/chan.h"
#include "../lib/timing.h"
#include <libwebsockets.h>
#include <string.h>

#define MAX_HEADERS 4096
#define MAX_CMD 256

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
 * Parse a command line and fill data in the pie_msg.
 * @param the pie_msg to initialize
 * @param the command string
 * @param the length of the command string, without the NULL terminator
 * @return 0 if message could be constructed from the command. Non zero
 *         otherwise.
 */
static  int parse_cmd_msg(struct pie_msg*, char*, size_t);

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

#if DEBUG > 0
                        printf("%s: [%s] received message %d (RTT %ldusec).\n",
                               __func__,
                               resp->token,
                               resp->type,
                               timing_dur_usec(&resp->t));
#endif

                        if (msg.len != sizeof(struct pie_msg))
                        {
                                printf("%s: invalid message received.\n",
                                       __func__);
                                goto msg_done;
                        }
                        
                        session = pie_sess_mgr_get(sess_mgr, resp->token);
                        if (session == NULL)
                        {
                                printf("%s: [%s] ERROR: not found.\n",
                                       __func__,
                                       resp->token);
                                goto msg_done;
                        }
                        switch (resp->type)
                        {
                        case PIE_MSG_LOAD_DONE:
                                /* On load done, a new image workspace 
                                   is created, store it in the session. */
                                session->img = resp->img;
                        case PIE_MSG_RENDER_DONE:
                                /* Update session with tx ready */
                                session->tx_ready = (PIE_TX_IMG | PIE_TX_HIST);
                                lws_callback_on_writable_all_protocol(srv->context, 
                                                                      &protocols[PIE_PROTO_IMG]);
                                lws_callback_on_writable_all_protocol(srv->context,
                                                                      &protocols[PIE_PROTO_HIST]);
                                break;
                        default:
                                printf("%s: [%s] invalid message: %d\n",
                                       __func__,
                                       resp->token,
                                       resp->type);
                        }

                msg_done:
                        pie_msg_free(msg.data);
                }
                else if (n != EAGAIN)
                {
                        printf("%s: Error reading channel %d\n", __func__, n);
                        srv->run = 0;
                }

        lws_service:
                /* Check for stop condition or img/hist is computed */
                /* If nothing to do, return after 100ms */
                status = lws_service(srv->context, 50);
                /* Reap sessions with inactivity for one hour */
                pie_sess_mgr_reap(sess_mgr, 60 * 60);
        }

        printf("Shutdown server.\n");
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
                /* Look for session */
                session = get_session(wsi);
                if (session == NULL)
                {
                        char cookie[128];
                        unsigned char* p = headers;
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
#if DEBUG > 0
                        printf("%s: [%s] Init session\n",
                               __func__,
                               session->token);

#endif
                        pie_sess_mgr_put(sess_mgr, session);
                        hn = p - headers;

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
                printf("%s: [%s] GET %s\n", 
                       __func__,
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
                n = lws_serve_http_file(wsi, url, mimetype, (char*)headers, hn);
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
        unsigned char buf[LWS_PRE + 1024];
        unsigned char* p = &buf[LWS_PRE];
        struct pie_sess* session;
        struct pie_ctx_img* ctx = (struct pie_ctx_img*)user;
        int ret = -1;
        int bw;

        if (user && reason != LWS_CALLBACK_ESTABLISHED)
        {
                session = pie_sess_mgr_get(sess_mgr, ctx->token);
        }

        switch (reason)
        {
        case LWS_CALLBACK_ESTABLISHED:
                /* Copy the session token, it is not available later on */
                if (session = get_session(wsi))
                {
                        strcpy(ctx->token, session->token);
                }
                else
                {
                        printf("%s: No session found\n", __func__);
                        return -1;
                }
                break;
        case LWS_CALLBACK_SERVER_WRITEABLE:
                if (!session)
                {
                        printf("%s: No session found\n", __func__);
                        return -1;
                }
                printf("%s: [%s] tx_ready: 0x%x\n",
                       __func__, 
                       session->token,
                       session->tx_ready);

                if (session->tx_ready)
                {
                        bw = lws_write(wsi, 
                                       session->img->proxy_out_rgba,
                                       session->img->proxy_out_len, 
                                       LWS_WRITE_BINARY);
                        if (bw < session->img->proxy_out_len)
                        {
                                printf("%s: [%s] ERROR write %d of %d\n",
                                       __func__,
                                       session->token,
                                       bw,
                                       session->img->proxy_out_len);
                                ret = -1;
                        }
                        session->tx_ready = 0;
                }
                break;
        case LWS_CALLBACK_RECEIVE:
                printf("%s: [%s] Got data: '%s'\n", 
                       __func__,
                       session->token,
                       in);

                break;
        default:
                break;
        }

        ret = 0;

        return ret;
}

static int cb_hist(struct lws* wsi,
                   enum lws_callback_reasons reason,
                   void* user,
                   void* in, 
                   size_t len)
{
        unsigned char buf[LWS_PRE + 1024];
        unsigned char* p = &buf[LWS_PRE];
        struct pie_sess* session;
        struct pie_ctx_hist* ctx = (struct pie_ctx_hist*)user;
        int ret = -1;

        if (user && reason != LWS_CALLBACK_ESTABLISHED)
        {
                session = pie_sess_mgr_get(sess_mgr, ctx->token);
        }

        switch (reason)
        {
        case LWS_CALLBACK_ESTABLISHED:
                /* Copy the session token, it is not available later on */
                if (session = get_session(wsi))
                {
                        strcpy(ctx->token, session->token);

                }
                else
                {
                        printf("%s: No session found\n", __func__);
                        return -1;
                }
                break;
        case LWS_CALLBACK_SERVER_WRITEABLE:

                if (!session)
                {
                        printf("%s: No session found\n", __func__);
                        return -1;
                }
                printf("%s [%s] tx_ready: 0x%x\n",
                       __func__, 
                       session->token,
                       session->tx_ready);
                break;
        case LWS_CALLBACK_RECEIVE:
                printf("%s [%s]Got data: '%s'\n", __func__, session->token, in);

                break;
        default:
                break;
        }

        ret = 0;

        return ret;
}

static int cb_cmd(struct lws* wsi,
                  enum lws_callback_reasons reason,
                  void* user,
                  void* in, 
                  size_t len)
{
        struct chan_msg envelope;
        struct pie_sess* session;
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
                if (session = get_session(wsi))
                {
                        strcpy(ctx->token, session->token);
                }
                else
                {
                        printf("%s: No session found\n", __func__);
                        return -1;
                }
                break;
        case LWS_CALLBACK_RECEIVE:
                msg = pie_msg_alloc();
                strncpy(msg->token, session->token, PIE_MSG_TOKEN_LEN);
                if (parse_cmd_msg(msg, (char*)in, len))
                {
                        printf("%s: failed to parse message: '%s'\n",
                               __func__, in);
                }
                else
                {
                        if (msg->type != PIE_MSG_LOAD && session->img == NULL)
                        {
                                printf("%s: [%s] No image loaded\n",
                                       __func__,
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
                                printf("%s: failed to write to chan\n",
                                       __func__);
                        }
                }

#if 0
                msg->type = PIE_MSG_SET_CONTRAST;
                msg->i1 = 50;

                if (chan_write(session->command, &envelope))
                {
                        printf("%s: failed to write to chan\n",
                               __func__);
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
	if (strcmp(&path[n - 3], ".js") == 0)
        {
		return "text/javascript";
        }

	return NULL;
}

static struct pie_sess* get_session(struct lws* wsi)
{
        char headers[MAX_HEADERS];
        struct pie_sess* session;
        int n = lws_hdr_total_length(wsi, WSI_TOKEN_HTTP_COOKIE);
        
        if (n == 0)
        {
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
                        return NULL;
                }
                t++;
                session = pie_sess_mgr_get(sess_mgr,
                                           t);
#if DEBUG > 1
                if (session)
                {
                        printf("Found session %s\n", t);
                }
#endif
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

static int parse_cmd_msg(struct pie_msg* msg, char* data, size_t len)
{
        char buf[MAX_CMD];
        char copy[MAX_CMD];
        char* lasts = buf;
        char* t;
        
        if (len >= MAX_CMD)
        {
                /* To long command */
                printf("%s: [%s] to long command: '%s'\n", 
                       __func__,
                       msg->token,
                       data);
                return -1;
        }

        /* Len does not include NULL */
        strncpy(copy, data, len);
        copy[len] = '\0';
        t = strtok_r(copy, " ", &lasts);

        if (t == NULL)
        {
                return -1;
        }

        if (strcmp(t, "LOAD") == 0)
        {
                msg->type = PIE_MSG_LOAD;
        }
        else if (strcmp(t, "CONTR") == 0)
        {
                /* CONTR {val} 
                   val = [0, 100] */
                t = strtok_r(NULL, " ", &lasts);
                if (t)
                {
                        char* p;
                        long v = strtol(t, &p, 10);
                        
                        if (t != p)
                        {
                                msg->type = PIE_MSG_SET_CONTRAST;
                                msg->f1 = v / 50.f;
                        }
                        else
                        {
                                printf("%s: [%s] not an int: '%s'\n",
                                       __func__,
                                       msg->token,
                                       data);
                                return -1;
                        }
                }
                else
                {
                        printf("%s: [%s] not a valid command '%s'\n",
                               __func__,
                               msg->token,
                               data);
                        return -1;
                }
        }
        else if (strcmp(t, "EXPOS") == 0)
        {
                printf("parse EXPOS\n");                
        }
        else if (strcmp(t, "HIGHL") == 0)
        {
                printf("parse HIGHL\n");                
        }
        else if (strcmp(t, "SHADO") == 0)
        {
                printf("parse SHADO\n");                
        }
        else if (strcmp(t, "WHITE") == 0)
        {
                printf("parse WHITE\n");                
        }
        else if (strcmp(t, "BLACK") == 0)
        {
                printf("parse BLACK\n");                
        }
        else if (strcmp(t, "CLARI") == 0)
        {
                printf("parse CLARI\n");                
        }
        else if (strcmp(t, "VIBRA") == 0)
        {
                printf("parse VIBRA\n");                
        }
        else if (strcmp(t, "SATUR") == 0)
        {
                printf("parse SATUR\n");                
        }
        else if (strcmp(t, "ROTAT") == 0)
        {
                printf("parse ROTAT\n");                
        }
        else if (strcmp(t, "CROP") == 0)
        {
                printf("parse CROP\n");                
        }
        else 
        {
                printf("%s: [%s] unknown command '%s'\n",
                       __func__,
                       msg->token,
                       data);
                return -1;
        }
                
#if 0
        while (t)
        {
                printf("Got token: %s\n", t);
                t = strtok_r(NULL, " ", &lasts);
        }
#endif
        return 0;
}
