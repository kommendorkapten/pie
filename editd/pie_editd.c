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
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#ifdef __sun
# include <note.h>
#else
# define NOTE(X)
#endif
#include "../editd/pie_editd_ws.h"
#include "../lib/chan.h"
#include "../lib/timing.h"
#include "../pie_types.h"
#include "../pie_log.h"
#include "../bm/pie_bm.h"
#include "../bm/pie_bm_dwn_smpl.h"
#include "../alg/pie_hist.h"
#include "../alg/pie_unsharp.h"
#include "pie_msg.h"
#include "pie_render.h"
#include "pie_wrkspc_mgr.h"

struct config
{
        char* lib_path;
        int wrk_cache_size;
};

static struct config config;
static struct pie_editd_ws server;
static struct pie_wrkspc_mgr* wrkspc_mgr;
static volatile int run;

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
 * @param NULL.
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
 * Callback. Update an image and renders the image.
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

        run = 1;
        config.lib_path = "test-images";
        config.wrk_cache_size = 10;
        server.directory = "assets";
        server.port = 8080;
        server.command = chan_create();
        server.response = chan_create();

        wrkspc_mgr = pie_wrkspc_mgr_create(config.lib_path,
                                           config.wrk_cache_size);

        /* interrupt handler thread */
        ok = pthread_create(&thr_int,
                            NULL,
                            &i_handler,
                            NULL);
        if (ok)
        {
                PIE_LOG("pthread_create:thr_int: %d", ok);
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
                PIE_LOG("pthread_create:thr_ev_loop: %d", ok);
                goto cleanup;

        }

        if (pie_editd_ws_start(&server))
        {
                PIE_ERR("Failed to start websocket server");
        }


        /* Run service loop */
        int ws_ok = 0;
        while (ws_ok >= 0 && run)
        {
                ws_ok = pie_editd_ws_service();
        }
        pie_editd_ws_stop();

        PIE_LOG("Shutdown main.");
        ok = 1;
        chan_close(server.command);
        chan_close(server.response);

        pthread_join(thr_int, &ret);
        if (ret)
        {
                ok = 0;
                PIE_ERR("Interrupt handler reported error.");
        }
        pthread_join(thr_ev_loop, &ret);
        if (ret)
        {
                ok = 0;
                PIE_ERR("Event loop reported error.");
        }

        if (ok)
        {
                PIE_LOG("All services exited cleanly.");
        }
cleanup:
        chan_destroy(server.command);
        chan_destroy(server.response);
        pie_wrkspc_mgr_destroy(wrkspc_mgr);

        return 0;
}

static void sig_h(int signum)
{
        if (signum == SIGINT)
        {
                run = 0;
        }
}

static void* i_handler(void* a)
{
        struct sigaction sa;
        void* ret = NULL;
        (void)a;

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
        PIE_DEBUG("Leaving.");
        run = 0;

        return ret;
}

static void* ev_loop(void* a)
{
        struct chan_msg msg;
        struct pie_editd_ws* s = (struct pie_editd_ws*)a;
        void* ret = NULL;

        PIE_DEBUG("Ready for messages.");

        while (run)
        {
                /* Select on multiple channels,
                   after a load issue the response will appear*/
                int n = chan_read(s->command, &msg, -1);

                PIE_TRACE("Got chan_read: %d", n);

                if (n == 0)
                {
                        struct pie_msg* cmd = (struct pie_msg*)msg.data;
                        struct timing t_proc;
                        int processed = 1;
                        enum pie_msg_type new = PIE_MSG_INVALID;

                        if (msg.len != sizeof(struct pie_msg))
                        {
                                PIE_WARN("invalid message received.");
                                continue;
                        }
                        PIE_DEBUG("[%s] received message %d.",
                               cmd->token,
                               cmd->type);

                        timing_start(&t_proc);
                        switch (cmd->type)
                        {
                        case PIE_MSG_LOAD:
                                new = cb_msg_load(cmd);
                                break;
                        case PIE_MSG_SET_COLOR_TEMP:
                        case PIE_MSG_SET_TINT:
                        case PIE_MSG_SET_EXSPOSURE:
                        case PIE_MSG_SET_CONTRAST:
                        case PIE_MSG_SET_HIGHL:
                        case PIE_MSG_SET_SHADOW:
                        case PIE_MSG_SET_WHITE:
                        case PIE_MSG_SET_BLACK:
                        case PIE_MSG_SET_CLARITY:
                        case PIE_MSG_SET_VIBRANCE:
                        case PIE_MSG_SET_SATURATION:
                        case PIE_MSG_SET_ROTATE:
                        case PIE_MSG_SET_SHARP:
                                new = cb_msg_render(cmd);
                                break;
                        default:
                                processed = 0;
                                PIE_WARN("Unknown message %d.",
                                         (int)cmd->type);
                        }
                        if (processed)
                        {
                                NOTE(EMPTY)
                                PIE_DEBUG("Processed message %d in %ldusec.",
                                          cmd->type,
                                          timing_dur_usec(&t_proc));
                        }
                        if (new != PIE_MSG_INVALID)
                        {
                                /* It is safe to re-transmit the
                                   message. It's the receiver's
                                   responsibility to free any message */
                                cmd->type = new;
                                if (chan_write(s->response, &msg))
                                {
                                        PIE_ERR("Failed to send response %d.",
                                                (int)cmd->type);
                                }
                        }
                        else
                        {
                                /* Message was not properly handled
                                   Free it. */
                                PIE_ERR("[%s] Failed to handle message %d",
                                        cmd->token,
                                        (int)cmd->type);
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
                        PIE_ERR("Channel reported error.");
                        ret = (void*)0x1L;
                        break;
                }
        }

        PIE_DEBUG("Leaving.");

        return ret;
}

/*
 * Create a new img work space.
 * Read file from path in buf.
 */
static enum pie_msg_type cb_msg_load(struct pie_msg* msg)
{
        struct timing t;
        struct timing t_l;
        int len;
        int res;
        int proxy_w;
        int proxy_h;
        int stride;
        int downsample = 0;

        timing_start(&t_l);

        if (msg->img)
        {
                /* Release earlier workspace */
                pie_wrkspc_mgr_release(wrkspc_mgr, msg->img);
        }

        msg->img = pie_wrkspc_mgr_acquire(wrkspc_mgr, msg->buf);

        if (msg->img == NULL)
        {
                return PIE_MSG_INVALID;
        }

        /* Init proxies */
        stride = msg->img->raw.row_stride;
        proxy_w = msg->i1;
        proxy_h = msg->i2;
        PIE_DEBUG("[%s] Request proxy [%d, %d] for image [%d, %d]",
                  msg->token,
                  proxy_w,
                  proxy_h,
                  msg->img->raw.width,
                  msg->img->raw.height);

        if (proxy_w < msg->img->raw.width || proxy_h < msg->img->raw.height)
        {
                PIE_DEBUG("[%s] Downsampling needed", msg->token);
                downsample = 1;
        }
        else
        {
                /* Do not do any up sampling */
                if (proxy_w > msg->img->raw.width)
                {
                        proxy_w = msg->img->raw.width;
                }
                if (proxy_h > msg->img->raw.height)
                {
                        proxy_h = msg->img->raw.height;
                }
                PIE_DEBUG("[%s] Image smaller than proxy, new proxy size [%d, %d]",
                          msg->token,
                          proxy_w,
                          proxy_h);
        }

        /* Allocate proxy images */
        if (downsample)
        {
                struct pie_unsharp_param up = {
                        .amount = 0.5f,
                        .radius = 0.5f,
                        .threshold = 2.0f
                };

                timing_start(&t);
                res = pie_bm_dwn_smpl(&msg->img->proxy,
                                      &msg->img->raw,
                                      proxy_w,
                                      proxy_h);
                if (res)
                {
                        abort();
                }
                PIE_DEBUG("[%s] Downsampled proxy in %ldusec",
                          msg->token,
                          timing_dur_usec(&t));

                /* Add some sharpening to mitigate any blur created
                   during downsampling. */
                timing_start(&t);
                res = pie_alg_unsharp(msg->img->proxy.c_red,
                                      msg->img->proxy.c_green,
                                      msg->img->proxy.c_blue,
                                      &up,
                                      msg->img->proxy.width,
                                      msg->img->proxy.height,
                                      msg->img->proxy.row_stride);
                if (res)
                {
                        abort();
                }
                PIE_DEBUG("[%s] Added post-downsampling sharpening in %ldusec",
                          msg->token,
                          timing_dur_usec(&t));
        }
        else
        {
                msg->img->proxy.width = proxy_w;
                msg->img->proxy.height = proxy_h;
                msg->img->proxy.color_type = PIE_COLOR_TYPE_RGB;
                pie_bm_alloc_f32(&msg->img->proxy);
                /* Copy raw to proxy */
                len = (int)(proxy_h * stride * sizeof(float));
                memcpy(msg->img->proxy.c_red, msg->img->raw.c_red, len);
                memcpy(msg->img->proxy.c_green, msg->img->raw.c_green, len);
                memcpy(msg->img->proxy.c_blue, msg->img->raw.c_blue, len);
        }

        /* Create a working copy of the proxy for editing */
        msg->img->proxy_out.width = msg->img->proxy.width;
        msg->img->proxy_out.height = msg->img->proxy.height;
        msg->img->proxy_out.color_type = PIE_COLOR_TYPE_RGB;
        pie_bm_alloc_f32(&msg->img->proxy_out);

        /* Copy proxy to proxy out */
        len = (int)(msg->img->proxy.height * msg->img->proxy.row_stride * sizeof(float));
        memcpy(msg->img->proxy_out.c_red, msg->img->proxy.c_red, len);
        memcpy(msg->img->proxy_out.c_green, msg->img->proxy.c_green, len);
        memcpy(msg->img->proxy_out.c_blue, msg->img->proxy.c_blue, len);

        /* Read from database to get settings */
        pie_img_init_settings(&msg->img->settings,
                              msg->img->proxy.width,
                              msg->img->proxy.height);

        /* Call render */
        res = pie_img_render(&msg->img->proxy_out,
                             NULL,
                             &msg->img->settings);
        assert(res == 0);

        /* Create histogram */
        pie_alg_hist_lum(&msg->img->hist, &msg->img->proxy_out);
        pie_alg_hist_rgb(&msg->img->hist, &msg->img->proxy_out);

        /* Issue a load cmd */
        PIE_DEBUG("[%s] Loaded proxy with size [%d, %d] in %ldusec",
                  msg->token,
                  msg->img->proxy.width,
                  msg->img->proxy.height,
                  timing_dur_usec(&t_l));

        return PIE_MSG_LOAD_DONE;
}

static enum pie_msg_type cb_msg_render(struct pie_msg* msg)
{
        int status = 0;
        enum pie_msg_type ret_msg = PIE_MSG_INVALID;
#if 0
        int resample = 0;
#endif

        switch (msg->type)
        {
        case PIE_MSG_SET_COLOR_TEMP:
                if (msg->f1 < -0.3f || msg->f1 > 0.3f)
                {
                        PIE_WARN("[%s] invalid color temp: %f.",
                                 msg->token,
                                 msg->f1);
                        status = -1;
                }
                else
                {
                        msg->img->settings.color_temp = msg->f1;
                        PIE_TRACE("[%s] Set color temp: %f.",
                                  msg->token,
                                  msg->img->settings.color_temp);
                }
                break;
        case PIE_MSG_SET_TINT:
                if (msg->f1 < -0.3f || msg->f1 > 0.3f)
                {
                        PIE_WARN("[%s] invalid tint: %f.",
                                 msg->token,
                                 msg->f1);
                        status = -1;
                }
                else
                {
                        msg->img->settings.tint = msg->f1;
                        PIE_TRACE("[%s] Set tint: %f.",
                                  msg->token,
                                  msg->img->settings.tint);
                }
                break;
        case PIE_MSG_SET_EXSPOSURE:
                if (msg->f1 < -5.0f || msg->f1 > 5.0f)
                {
                        PIE_WARN("[%s] invalid exposure: %f.",
                                 msg->token,
                                 msg->f1);
                        status = -1;
                }
                else
                {
                        msg->img->settings.exposure = msg->f1;
                        PIE_TRACE("[%s] Set exposure: %f.",
                                  msg->token,
                                  msg->img->settings.exposure);
                }
                break;
        case PIE_MSG_SET_CONTRAST:
                if (msg->f1 < 0.0f || msg->f1 > 2.0f)
                {
                        PIE_WARN("[%s] invalid contrast: %f.",
                                 msg->token,
                                 msg->f1);
                        status = -1;
                }
                else
                {
                        msg->img->settings.contrast = msg->f1;
                        PIE_TRACE("[%s] Set contrast: %f.",
                                  msg->token,
                                  msg->img->settings.contrast);
                }
                break;
        case PIE_MSG_SET_HIGHL:
                if (msg->f1 < -1.0f || msg->f1 > 1.0f)
                {
                        PIE_WARN("[%s] invalid highlight: %f.",
                                 msg->token,
                                 msg->f1);
                        status = -1;
                }
                else
                {
                        msg->img->settings.highlights = msg->f1;
                        PIE_TRACE("[%s] Set highlights: %f.",
                                  msg->token,
                                  msg->img->settings.highlights);
                }
                break;
        case PIE_MSG_SET_SHADOW:
                if (msg->f1 < -1.0f || msg->f1 > 1.0f)
                {
                        PIE_WARN("[%s] invalid shadow: %f.",
                                 msg->token,
                                 msg->f1);
                        status = -1;
                }
                else
                {
                        msg->img->settings.shadows = msg->f1;
                        PIE_TRACE("[%s] Set shadows: %f.",
                                  msg->token,
                                  msg->img->settings.shadows);
                }
                break;
        case PIE_MSG_SET_WHITE:
                if (msg->f1 < -1.0f || msg->f1 > 1.0f)
                {
                        PIE_WARN("[%s] invalid white: %f.",
                                 msg->token,
                                 msg->f1);
                        status = -1;
                }
                else
                {
                        msg->img->settings.white = msg->f1;
                        PIE_TRACE("[%s] Set white: %f.",
                                  msg->token,
                                  msg->img->settings.white);
                }
                break;
        case PIE_MSG_SET_BLACK:
                if (msg->f1 < -1.0f || msg->f1 > 1.0f)
                {
                        PIE_WARN("[%s] invalid black: %f.",
                                 msg->token,
                                 msg->f1);
                        status = -1;
                }
                else
                {
                        msg->img->settings.black = msg->f1;
                        PIE_TRACE("[%s] Set black: %f.",
                                  msg->token,
                                  msg->img->settings.black);
                }
                break;
        case PIE_MSG_SET_CLARITY:
                if (msg->f1 < -1.0f || msg->f1 > 1.0f)
                {
                        PIE_WARN("[%s] invalid clarity: %f.",
                                 msg->token,
                                 msg->f1);
                        status = -1;
                }
                else
                {
                        msg->img->settings.clarity.amount = msg->f1;
                        PIE_TRACE("[%s] Set clarity: %f.",
                                  msg->token,
                                  msg->img->settings.clarity.amount);
                }
                break;
        case PIE_MSG_SET_VIBRANCE:
                if (msg->f1 < -1.0f || msg->f1 > 1.0f)
                {
                        PIE_WARN("[%s] invalid vibrance: %f.",
                                 msg->token,
                                 msg->f1);
                        status = -1;
                }
                else
                {
                        msg->img->settings.vibrance = msg->f1;
                        PIE_TRACE("[%s] Set vibrance: %f.",
                                  msg->token,
                                  msg->img->settings.vibrance);
                }
                break;
        case PIE_MSG_SET_SATURATION:
                if (msg->f1 < 0.0f || msg->f1 > 2.0f)
                {
                        PIE_WARN("[%s] invalid saturation: %f.",
                                 msg->token,
                                 msg->f1);
                        status = -1;
                }
                else
                {
                        msg->img->settings.saturation = msg->f1;
                        PIE_TRACE("[%s] Set saturation: %f.",
                                  msg->token,
                                  msg->img->settings.saturation);
                }
                break;
        case PIE_MSG_SET_ROTATE:
                PIE_WARN("[%s] Not implemented yet %d.",
                         msg->token,
                         (int)msg->type);
                status = -1;
                break;
        case PIE_MSG_SET_SHARP:
                if (msg->f1 < 0.0f || msg->f1 > 3.0f ||
                    msg->f2 < 0.1f || msg->f2 > 10.0f ||
                    msg->f3 < 0.0f || msg->f3 > 20.0f)
                {
                        PIE_WARN("[%s] invalid sharpening: %f %f %f.",
                                 msg->token,
                                 msg->f1,
                                 msg->f2,
                                 msg->f3);
                        status = -1;
                }
                else
                {
                        msg->img->settings.sharpening.amount = msg->f1;
                        msg->img->settings.sharpening.radius = msg->f2;
                        msg->img->settings.sharpening.threshold = msg->f3;
                        PIE_TRACE("[%s] Set sharpening: %f %f %f.",
                                  msg->token,
                                  msg->img->settings.sharpening.amount,
                                  msg->img->settings.sharpening.radius,
                                  msg->img->settings.sharpening.threshold);
                }
                break;
        default:
                PIE_WARN("[%s] Invalid message: %d.",
                         msg->token,
                         (int)msg->type);
                status = -1;
        }

#if 0
        if (resample)
        {
                PIE_ERR("Resample not implemeted");
                abort();
        }
#endif

        /* New parameters are set. Update the current workspace with the
         * following steps:
         * 1 create a new copy of the stored proxy.
         * 2 extract histogram data.
         */
        if (status == 0)
        {
                struct timing t1;
                struct pie_bitmap_f32rgb* org = &msg->img->proxy;
                struct pie_bitmap_f32rgb* new = &msg->img->proxy_out;
                size_t len = org->height * org->row_stride * sizeof(float);
                int r_ok;

                /* Copy fresh proxy */
                timing_start(&t1);
                memcpy(new->c_red, org->c_red, len);
                memcpy(new->c_green, org->c_green, len);
                memcpy(new->c_blue, org->c_blue, len);
                PIE_DEBUG(" Reset proxy:           %8ldusec",
                          timing_dur_usec(&t1));

                r_ok = pie_img_render(new,
                                      NULL,
                                      &msg->img->settings);
                assert(r_ok == 0);

                /* Create histogram */
                timing_start(&t1);
                pie_alg_hist_lum(&msg->img->hist,
                                 new);
                pie_alg_hist_rgb(&msg->img->hist,
                                 new);
                PIE_DEBUG(" Created histogram:     %8ldusec",
                          timing_dur_usec(&t1));

                ret_msg = PIE_MSG_RENDER_DONE;
        }

        return ret_msg;
}
