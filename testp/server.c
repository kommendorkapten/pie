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

#include "../wsrv/pie_server.h"
#include "../lib/chan.h"
#include "../lib/timing.h"
#include "../msg/pie_msg.h"
#include "../pie_types.h"
#include "../pie_bm.h"
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

struct pie_server server;

/**
 * Signal handler.
 * @param the signum.
 * @return void.
 */
static void sig_h(int);

/**
 * Main event handling loop
 * @param struct server.
 * @return NULL if successfull.
 */
static void* ev_loop(void*);

/**
 * Interrupt handler.
 * @param struct server.
 * @return NULL if successfull.
 */
static void* i_handler(void*);

/**
 * Callback. Loads an image.
 * Path is provided in buf.
 * @param message to act on.
 * @return if the message should be retransmit, the new msg type,
 *         if message should be dropped, PIE_MSG_INVALID.
 */
static enum pie_msg_type cb_msg_load(struct pie_msg*);

/**
 * Callbak. Update an image and renders the image.
 * @param message to acton.
 * @return if the message should be retransmit, the new msg type,
 *         if message should be dropped, PIE_MSG_INVALID.
 */
static enum pie_msg_type cb_msg_render(struct pie_msg*);

int main(void)
{
        pthread_t thr_ev_loop;
        pthread_t thr_int;
        sigset_t b_sigset;
        void* ret;
        int ok;

        server.context_root = "assets";
        server.port = 8080;
        server.command = chan_create();
        server.response = chan_create();

        /* interrupt handler thread */
        ok = pthread_create(&thr_int,
                            NULL,
                            &i_handler,
                            (void*) &server);
        if (ok)
        {
                printf("main::pthread_create:thr_int: %d\n", ok);
                goto cleanup;

        }

        /* Block all signals during thread creating */
        sigfillset(&b_sigset);
        pthread_sigmask(SIG_BLOCK, &b_sigset, NULL);

        /* Create a pool of worker threads for IO */
        /* Create a pool of worker threads for render */

        /* event loop thread */
        ok = pthread_create(&thr_ev_loop,
                            NULL,
                            &ev_loop,
                            (void*) &server);
        if (ok)
        {
                printf("main::pthread_create:thr_ev_loop: %d\n", ok);
                goto cleanup;

        }

        server.run = 1;
        if (start_server(&server))
        {
                printf("Failed\n");
        }

        
        printf("Shutdown main.\n");
        ok = 1;
        chan_close(server.command);
        chan_close(server.response);
        
        pthread_join(thr_int, &ret);
        if (ret)
        {
                ok = 0;
                printf("Interrupt handler reported error\n");
        }
        pthread_join(thr_ev_loop, &ret);
        if (ret)
        {
                ok = 0;
                printf("Event loop reported error\n");
        }

        if (ok)
        {
                printf("All services exited cleanly.\n");
        }
cleanup:
        chan_destroy(server.command);
        chan_destroy(server.response);

        return 0;
}

static void sig_h(int signum)
{
        printf("IN SIG_H %d\n", signum);
        if (signum == SIGINT)
        {
                server.run = 0;
        }
}

static void* i_handler(void* a)
{
        struct sigaction sa;
        struct pie_server* s = (struct pie_server*)a;
        void* ret = NULL;

        /* Set up signal handler */
        sa.sa_handler = &sig_h;
        sa.sa_flags = 0;
        sigfillset(&sa.sa_mask);
        if (sigaction(SIGINT, &sa, NULL))
        {
                perror("i_handler::sigaction");
                return (void*)0x1L;
        }
        
        pause();
        printf("%s: leaving.\n", __func__);

        return ret;
}

static void* ev_loop(void* a)
{
        struct chan_msg msg;
        struct pie_server* s = (struct pie_server*)a;
        void* ret = NULL;

#if DEBUG > 0
        printf("%s: ready for messages.\n", __func__);
#endif

        while (s->run)
        {
                /* Select on multiple channels,
                   after a load issue the response will appear*/
                int n = chan_read(s->command, &msg, -1);
#if DEBUG > 1
                printf("%s: Got chan_read: %d\n", __func__, n);
#endif
                if (n == 0)
                {
                        struct pie_msg* cmd = (struct pie_msg*)msg.data;
                        struct timing t;
                        int processed = 1;
                        enum pie_msg_type new = PIE_MSG_INVALID;

                        if (msg.len != sizeof(struct pie_msg))
                        {
                                printf("%s: invalid message received.\n",
                                       __func__);
                                continue;
                        }
#if DEBUG > 0
                        printf("%s: [%s] received message %d.\n",
                               __func__,
                               cmd->token,
                               cmd->type);
#endif
                        timing_start(&t);
                        switch (cmd->type)
                        {
                        case PIE_MSG_LOAD:
                                new = cb_msg_load(cmd);
                                break;
                        case PIE_MSG_SET_CONTRAST:
                        case PIE_MSG_SET_ESPOSURE:
                        case PIE_MSG_SET_HIGHL:
                        case PIE_MSG_SET_SHADOW:
                        case PIE_MSG_SET_WHITE:
                        case PIE_MSG_SET_BLACK:
                        case PIE_MSG_SET_CLARITY:
                        case PIE_MSG_SET_VIBRANCE:
                        case PIE_MSG_SET_SATURATION:
                        case PIE_MSG_SET_ROTATE:
                                new = cb_msg_render(cmd);
                                break;
                        default:
                                processed = 0;
                                printf("%s: unknown message %d.\n", 
                                       __func__,
                                       cmd->type);
                        }
                        if (processed)
                        {
                                printf("%s: processed message %d in %ldusec\n",
                                       __func__,
                                       cmd->type,
                                       timing_dur_usec(&t));
                        }
                        if (new != PIE_MSG_INVALID)
                        {
                                /* It is safe to re-transmit the
                                   message. It's the receiver's
                                   responsibility to free any message */
                                cmd->type = new;
                                if (chan_write(s->response, &msg))
                                {
                                        printf("%s: failed to send response %d\n",
                                               __func__, 
                                               cmd->type);
                                }
                        }
                        else
                        {
                                /* Message was not properly handled
                                   Free it. */
                                pie_msg_free(cmd);
                        }
                }
                else if (n == EBADF)
                {
                        /* Channel closed */
                        break;
                }
                else if (n != EAGAIN)
                {
                        printf("ev_loop: channel reported error.\n");
                        ret = (void*)0x1L;
                        break;
                }
        }

#if DEBUG > 0
        printf("%s: leaving.\n", __func__);
#endif

        return ret;
}

/*
 * Create a new img work space.
 * Read file from path in buf.
 */
static enum pie_msg_type cb_msg_load(struct pie_msg* msg)
{
        unsigned char* p;
        int status;
        int width = 1024;
        int height = 100;
        unsigned int row_stride;
        assert(msg->img == NULL);

        /* HACK */
        msg->img = malloc(sizeof(struct pie_img_workspace));
        memset(msg->img, 0, sizeof(struct pie_img_workspace));
        msg->img->raw.width = width;
        msg->img->raw.height = height;
        msg->img->raw.color_type = PIE_COLOR_TYPE_RGB;
        bm_alloc_f32(&msg->img->raw); 
        msg->img->proxy_out_len = msg->img->raw.width * msg->img->raw.height * 4;
        msg->img->buf = malloc(msg->img->proxy_out_len + PROXY_RGBA_OFF);
        msg->img->proxy_out_rgba = msg->img->buf + PROXY_RGBA_OFF;
        row_stride = msg->img->raw.row_stride;
        p = msg->img->proxy_out_rgba;

        /* Init with linear gradient */
        for (int y = 0; y < msg->img->raw.height; y++)
        {
                for (int x = 0; x < msg->img->raw.width; x++)
                {
                        int c = x / 4;
                        float f = c / 255.0f;

                        msg->img->raw.c_red[y * row_stride + x] = f;
                        msg->img->raw.c_green[y * row_stride + x] = f;
                        msg->img->raw.c_blue[y * row_stride + x] = f;
                }
        }        

        /* Encode proxy rgba */
        for (int y = 0; y < msg->img->raw.height; y++)
        {
                for (int x = 0; x < msg->img->raw.width; x++)
                {
                        /* convert to sse */
                        unsigned char r, g, b;
                        
                        r = (unsigned char)(msg->img->raw.c_red[y * row_stride + x] * 255.0f);
                        g = (unsigned char)(msg->img->raw.c_green[y * row_stride + x] * 255.0f);
                        b = (unsigned char)(msg->img->raw.c_blue[y * row_stride + x] * 255.0f);
                        
                        *p++ = r;
                        *p++ = g;
                        *p++ = b;
                        *p++ = 255;
                }
        }

        /* Issue a load cmd */

        return PIE_MSG_LOAD_DONE;
}

static enum pie_msg_type cb_msg_render(struct pie_msg* msg)
{
        int status = 0;
        
        switch (msg->type)
        {
        case PIE_MSG_SET_CONTRAST:
                if (msg->f1 < 0.0f || msg->f1 > 2.0f)
                {
                        printf("%s: [%s] invalid contrast: %f\n",
                               __func__,
                               msg->token,
                               msg->f1);
                        status = -1;
                }
                else
                {
                        msg->img->settings.contrast = msg->f1;
                }
                break;
        case PIE_MSG_SET_ESPOSURE:
        case PIE_MSG_SET_HIGHL:
        case PIE_MSG_SET_SHADOW:
        case PIE_MSG_SET_WHITE:
        case PIE_MSG_SET_BLACK:
        case PIE_MSG_SET_CLARITY:
        case PIE_MSG_SET_VIBRANCE:
        case PIE_MSG_SET_SATURATION:
        case PIE_MSG_SET_ROTATE:
                printf("%s: [%s] Not implemented yet %d\n",
                       __func__,
                       msg->token,
                       msg->type);
                status = -1;
                break;
        default:
                printf("%s: [%s] Invalid message: %d\n",
                       __func__,
                       msg->token,
                       msg->type);
                status = -1;
        }

        if (status == 0)
        {
                return PIE_MSG_RENDER_DONE;
        }

        return PIE_MSG_INVALID;
}
