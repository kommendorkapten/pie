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
#include "pie_msg.h"
#include "pie_render.h"
#include "pie_wrkspc_mgr.h"
#include "pie_editd_ws.h"
#include "../lib/chan.h"
#include "../lib/timing.h"
#include "../pie_types.h"
#include "../pie_log.h"
#include "../bm/pie_bm.h"
#include "../alg/pie_unsharp.h"
#include "../pie_id.h"
#include "../cfg/pie_cfg.h"
#include "../mq_msg/pie_mq_msg.h"
#include "../lib/s_queue.h"
#include "../encoding/pie_json.h"
#include "../dm/pie_dev_params.h"
#include "../exif/pie_exif.h"

struct config
{
        int wrk_cache_size;
};

static struct config config;
static struct pie_editd_ws server;
static struct pie_wrkspc_mgr* wrkspc_mgr;
static struct q_producer* media_q;
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
 * Callback. Updates the chosen viewport.
 * @param message with new viewport coordinaes.
 * @return if the message should be retransmit, the new msg type.
 *         if message should be dropped, PIE_INVALID_MSG.
 */
static enum pie_msg_type cb_msg_viewp(struct pie_msg*);

/**
 * Callback. Update an image and renders the image.
 * @param message to acton.
 * @return if the message should be retransmit, the new msg type,
 *         if message should be dropped, PIE_MSG_INVALID.
 */
static enum pie_msg_type cb_msg_render(struct pie_msg*);

/**
 * Send message to mediad for persistance.
 * @param mob id
 * @param settigs to store.
 * @return void.
 */
static void store_settings(pie_id, const struct pie_dev_settings*);

/**
 * Create and downsample a proxy.
 * @param the target bitmap. Must be unallocated.
 * @param source bitmap.
 * @param requested proxy max width.
 * @param requested proxy max height.
 * @return void
 */
static void create_proxy(struct pie_bitmap_f32rgb*,
                         struct pie_bitmap_f32rgb*,
                         int,
                         int);

/**
 * Parse control points and store them in the provided curve.
 * Control points are in the following format:
 * -587,-343;0,0;588,343;1000,1000;1413,1657
 * The numbers are real values, scaled with 1000.
 * @param pointer to curve struct to store the points.
 * @param null terminated string in correct format.
 * @return 0 if data was successfully parsed.
 */
static int parse_curve_pnts(struct pie_curve*, char*);

int main(void)
{
        pthread_t thr_ev_loop;
        pthread_t thr_int;
        sigset_t b_sigset;
        void* ret;
        int ok;

        /* Load storages */
        if (pie_cfg_load(PIE_CFG_PATH))
        {
                PIE_ERR("Failed to read conf");
                return 1;
        }

        /* Init media queue */
        media_q = q_new_producer(QUEUE_INTRA_HOST);
        if (media_q == NULL)
        {
                PIE_ERR("Could not initiate media queue");
                return 1;
        }
        ok = media_q->init(media_q->this, Q_UPDATE_META);
        if (ok)
        {
                PIE_ERR("Can not connect to '%s'", Q_UPDATE_META);
                return 1;
        }

        run = 1;
        config.wrk_cache_size = 10;
        server.directory = "assets";
        server.port = 8080;
        server.command = chan_create();
        server.response = chan_create();

        wrkspc_mgr = pie_wrkspc_mgr_create(pie_cfg_get_db(),
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
                        case PIE_MSG_VIEWPORT:
                                new = cb_msg_viewp(cmd);
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
                        case PIE_MSG_SET_CURVE:
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

                                if (cmd->type == PIE_MSG_RENDER_DONE)
                                {
                                        /* Store update development settings */
                                        timing_start(&t_proc);
                                        cmd->wrkspc->settings.version = 1;
                                        store_settings(cmd->wrkspc->mob_id,
                                                       &cmd->wrkspc->settings);
                                        PIE_DEBUG("Stored dev settings in %ldusec.",
                                                  timing_dur_usec(&t_proc));
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
        struct pie_dev_params settings_json;
        struct timing t;
        struct pie_bitmap_f32rgb* raw;
        struct pie_bitmap_f32rgb* proxy;
        struct pie_bitmap_f32rgb* proxy_out;
        pie_id id;
        int len;
        int res;
        int proxy_w;
        int proxy_h;
        int stride;
        int downsample = 0;

        timing_start(&t);

        if (msg->wrkspc)
        {
                /* Release earlier workspace */
                pie_wrkspc_mgr_release(wrkspc_mgr, msg->wrkspc);
        }

        PIE_DEBUG("Load id %s", msg->buf);
        id = pie_id_from_str(msg->buf);
        PIE_DEBUG("Parsed id: %ld\n", id);
        if (id == 0 || !PIE_ID_IS_MOB(id))
        {
                return PIE_MSG_INVALID;
        }

        msg->wrkspc = pie_wrkspc_mgr_acquire(wrkspc_mgr, id);
        if (msg->wrkspc == NULL)
        {
                return PIE_MSG_INVALID;
        }

        raw = &msg->wrkspc->raw;
        proxy = &msg->wrkspc->proxy;
        proxy_out = &msg->wrkspc->proxy_out;

        /* Init proxies */
        stride = raw->row_stride;
        proxy_w = msg->i1;
        proxy_h = msg->i2;
        PIE_DEBUG("[%s] Request proxy [%d, %d] for image [%d, %d]",
                  msg->token,
                  proxy_w,
                  proxy_h,
                  raw->width,
                  raw->height);

        switch (msg->wrkspc->exif.ped_orientation)
        {
        case PIE_EXIF_ORIENTATION_CW90:
        case PIE_EXIF_ORIENTATION_CW270:
                PIE_LOG("Image is in portrait orientation.");
                int tmp = proxy_w;
                proxy_w = proxy_h;
                proxy_h = tmp;
        }

        if (proxy_w < raw->width ||
            proxy_h < raw->height)
        {
                PIE_DEBUG("[%s] Downsampling needed", msg->token);
                downsample = 1;
        }
        else
        {
                /* Do not do any up sampling */
                if (proxy_w > raw->width)
                {
                        proxy_w = raw->width;
                }
                if (proxy_h > raw->height)
                {
                        proxy_h = raw->height;
                }
                PIE_DEBUG("[%s] Image smaller than proxy, new proxy size [%d, %d]",
                          msg->token,
                          proxy_w,
                          proxy_h);
        }

        /* Allocate proxy images */
        if (downsample)
        {
                create_proxy(proxy,
                             raw,
                             proxy_w,
                             proxy_h);
        }
        else
        {
                proxy->width = proxy_w;
                proxy->height = proxy_h;
                proxy->color_type = PIE_COLOR_TYPE_RGB;
                pie_bm_alloc_f32(proxy);
                /* Copy raw to proxy */
                len = (int)(proxy_h * stride * sizeof(float));
                memcpy(proxy->c_red, raw->c_red, len);
                memcpy(proxy->c_green, raw->c_green, len);
                memcpy(proxy->c_blue, raw->c_blue, len);
        }

        /* Create a working copy of the proxy for editing */
        proxy_out->width = proxy->width;
        proxy_out->height = proxy->height;
        proxy_out->color_type = PIE_COLOR_TYPE_RGB;
        pie_bm_alloc_f32(proxy_out);

        /* Copy proxy to proxy out */
        len = (int)(proxy->height * proxy->row_stride * sizeof(float));
        memcpy(proxy_out->c_red, proxy->c_red, len);
        memcpy(proxy_out->c_green, proxy->c_green, len);
        memcpy(proxy_out->c_blue, proxy->c_blue, len);

        /* Load stored development settings. First
           First initialize the settings, as the settings are stored
           as a JSON string. This way we can be future compatible,
           if the format stored in the DB is older, only the relevat
           changes are merged. */
        pie_dev_init_settings(&msg->wrkspc->settings,
                              proxy->width,
                              proxy->height);

        settings_json.pdp_mob_id = id;
        res = pie_dev_params_read(pie_cfg_get_db(), &settings_json);
        if (res == 0)
        {
                res = pie_dec_json_settings(&msg->wrkspc->settings,
                                            settings_json.pdp_settings);
                if (res)
                {
                        PIE_ERR("Broken dev settings stored in db for MOB %ld",
                                id);
                        pie_dev_init_settings(&msg->wrkspc->settings,
                                              proxy->width,
                                              proxy->height);
                }
                /* Free any resources */
                pie_dev_params_release(&settings_json);

                /* Convert to internal format */
                pie_dev_set_to_int_fmt(&msg->wrkspc->settings);
        }
        else if (res < 0)
        {
                PIE_ERR("Database error: %d", res);
        }

        /* Call render */
        res = pie_dev_render(proxy_out,
                             NULL,
                             &msg->wrkspc->settings);
        assert(res == 0);

        /* Issue a load cmd */
        PIE_DEBUG("[%s] Loaded proxy with size [%d, %d] in %ldusec",
                  msg->token,
                  proxy->width,
                  proxy->height,
                  timing_dur_usec(&t));

        return PIE_MSG_LOAD_DONE;
}

static enum pie_msg_type cb_msg_viewp(struct pie_msg* msg)
{
        struct pie_bitmap_f32rgb* raw = &msg->wrkspc->raw;
        struct pie_bitmap_f32rgb* proxy = &msg->wrkspc->proxy;
        struct pie_bitmap_f32rgb* proxy_out = &msg->wrkspc->proxy_out;
        int x0 = msg->i1;
        int y0 = msg->i2;
        int x1 = msg->i3;
        int y1 = msg->i4;
        int t_w = msg->i5;
        int t_h = msg->i6;
        int len;
        int res;
        float scale;
        char new_proxy = 0;

        PIE_DEBUG("[%s]Requested viewport to (%d, %d) (%d, %d) to target size %d x %d",
                  msg->token,
                  x0,
                  y0,
                  x1,
                  y1,
                  t_w,
                  t_h);

        /* Compensate for any rotation. Coordinates are provided in the
           image's oriented space. Not the image's physical space. */
        switch (msg->wrkspc->exif.ped_orientation)
        {
        case PIE_EXIF_ORIENTATION_CW90:
        case PIE_EXIF_ORIENTATION_CW270:
                PIE_LOG("Image is in portrait orientation.");
                int tmp;

                tmp = t_w;
                t_w = t_h;
                t_h = tmp;

                tmp = x0;
                x0 = y0;
                y0 = tmp;

                tmp = x1;
                x1 = y1;
                y1 = tmp;

        }

        /* Validate arguments */
        if (x0 < 0)
        {
                x0 = 0;
        }
        if (y0 < 0)
        {
                y0 = 0;
        }
        if (x1 > raw->width)
        {
                x1 = raw->width;
        }
        if (y1 > raw->height)
        {
                y1 = raw->height;
        }

        if (x1 < 0 || y1 < 0)
        {
                /* full image is requested */
                x1 = raw->width;
                y1 = raw->height;
        }

        scale = (float)t_w / (float)(x1 - x0);

        PIE_DEBUG("[%s]Image space viewport to (%d, %d) (%d, %d) to target size %d x %d (scale: %f)",
                  msg->token,
                  x0,
                  y0,
                  x1,
                  y1,
                  t_w,
                  t_h,
                  scale);

        if (scale < 1.00000f)
        {
                /* custom scale is not yet supported. */
                pie_bm_free_f32(proxy);
                create_proxy(proxy, raw, t_w, t_h);
                if (t_w != proxy->width ||
                    t_h != proxy->height)
                {
                        PIE_LOG("Reallocating proxies to (%dx%d), was (%dx%d)",
                                proxy->width,
                                proxy->height,
                                proxy_out->width,
                                proxy_out->height);
                        new_proxy = 1;
                        /* Reallocate proxy out */
                        pie_bm_free_f32(proxy_out);

                        proxy_out->width = proxy->width;
                        proxy_out->height = proxy->height;
                        proxy_out->color_type = PIE_COLOR_TYPE_RGB;
                        pie_bm_alloc_f32(proxy_out);
                }
        }
        else
        {
                if (t_w != proxy->width ||
                    t_h != proxy->height)
                {
                        PIE_LOG("Reallocating proxies to (%dx%d), was (%dx%d)",
                                t_w,
                                t_h,
                                proxy_out->width,
                                proxy_out->height);
                        new_proxy = 1;

                        /* reallocate proxies */
                        pie_bm_free_f32(proxy);
                        pie_bm_free_f32(proxy_out);

                        proxy->width = t_w;
                        proxy->height = t_h;
                        proxy->color_type = PIE_COLOR_TYPE_RGB;
                        proxy_out->width = t_w;
                        proxy_out->height = t_h;
                        proxy_out->color_type = PIE_COLOR_TYPE_RGB;

                        pie_bm_alloc_f32(proxy);
                        pie_bm_alloc_f32(proxy_out);
                }

                len = t_w * (int)sizeof(float);
                /* Copy a non scaled portion */
                switch (msg->wrkspc->exif.ped_orientation)
                {
                case PIE_EXIF_ORIENTATION_CW90:
                        x0 = raw->width - t_w - x0;
                        break;
                case PIE_EXIF_ORIENTATION_CW180:
                        x0 = raw->width - t_w - x0;
                        y0 = raw->height - t_h - y0;
                        break;
                case PIE_EXIF_ORIENTATION_CW270:
                        y0 = raw->height - t_h - y0;
                        break;
                }

                assert(x0 + t_w <= raw->width);
                assert(y0 + t_h <= raw->height);
                for (int i = 0; i < t_h; i++)
                {
                        memcpy(proxy->c_red + i * proxy->row_stride,
                               raw->c_red + (i + y0) * raw->row_stride + x0,
                               len);
                        memcpy(proxy->c_green + i * proxy->row_stride,
                               raw->c_green + (i + y0) * raw->row_stride + x0,
                               len);
                        memcpy(proxy->c_blue + i * proxy->row_stride,
                               raw->c_blue + (i + y0) * raw->row_stride + x0,
                               len);
                }
        }

        /* Copy proxy to proxy out */
        len = (int)(proxy->height * proxy->row_stride * sizeof(float));
        memcpy(proxy_out->c_red, proxy->c_red, len);
        memcpy(proxy_out->c_green, proxy->c_green, len);
        memcpy(proxy_out->c_blue, proxy->c_blue, len);

        res = pie_dev_render(proxy_out,
                             NULL,
                             &msg->wrkspc->settings);
        assert(res == 0);

        if (new_proxy)
        {
                return PIE_MSG_NEW_PROXY_DIM;
        }
        return PIE_MSG_RENDER_DONE;
}

static enum pie_msg_type cb_msg_render(struct pie_msg* msg)
{
        int ok = 0;
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
                        ok = -1;
                }
                else
                {
                        msg->wrkspc->settings.color_temp = msg->f1;
                        PIE_TRACE("[%s] Set color temp: %f.",
                                  msg->token,
                                  msg->wrkspc->settings.color_temp);
                }
                break;
        case PIE_MSG_SET_TINT:
                if (msg->f1 < -0.3f || msg->f1 > 0.3f)
                {
                        PIE_WARN("[%s] invalid tint: %f.",
                                 msg->token,
                                 msg->f1);
                        ok = -1;
                }
                else
                {
                        msg->wrkspc->settings.tint = msg->f1;
                        PIE_TRACE("[%s] Set tint: %f.",
                                  msg->token,
                                  msg->wrkspc->settings.tint);
                }
                break;
        case PIE_MSG_SET_EXSPOSURE:
                if (msg->f1 < -5.0f || msg->f1 > 5.0f)
                {
                        PIE_WARN("[%s] invalid exposure: %f.",
                                 msg->token,
                                 msg->f1);
                        ok = -1;
                }
                else
                {
                        msg->wrkspc->settings.exposure = msg->f1;
                        PIE_TRACE("[%s] Set exposure: %f.",
                                  msg->token,
                                  msg->wrkspc->settings.exposure);
                }
                break;
        case PIE_MSG_SET_CONTRAST:
                if (msg->f1 < 0.0f || msg->f1 > 2.0f)
                {
                        PIE_WARN("[%s] invalid contrast: %f.",
                                 msg->token,
                                 msg->f1);
                        ok = -1;
                }
                else
                {
                        msg->wrkspc->settings.contrast = msg->f1;
                        PIE_TRACE("[%s] Set contrast: %f.",
                                  msg->token,
                                  msg->wrkspc->settings.contrast);
                }
                break;
        case PIE_MSG_SET_HIGHL:
                if (msg->f1 < -1.0f || msg->f1 > 1.0f)
                {
                        PIE_WARN("[%s] invalid highlight: %f.",
                                 msg->token,
                                 msg->f1);
                        ok = -1;
                }
                else
                {
                        msg->wrkspc->settings.highlights = msg->f1;
                        PIE_TRACE("[%s] Set highlights: %f.",
                                  msg->token,
                                  msg->wrkspc->settings.highlights);
                }
                break;
        case PIE_MSG_SET_SHADOW:
                if (msg->f1 < -1.0f || msg->f1 > 1.0f)
                {
                        PIE_WARN("[%s] invalid shadow: %f.",
                                 msg->token,
                                 msg->f1);
                        ok = -1;
                }
                else
                {
                        msg->wrkspc->settings.shadows = msg->f1;
                        PIE_TRACE("[%s] Set shadows: %f.",
                                  msg->token,
                                  msg->wrkspc->settings.shadows);
                }
                break;
        case PIE_MSG_SET_WHITE:
                if (msg->f1 < -1.0f || msg->f1 > 1.0f)
                {
                        PIE_WARN("[%s] invalid white: %f.",
                                 msg->token,
                                 msg->f1);
                        ok = -1;
                }
                else
                {
                        msg->wrkspc->settings.white = msg->f1;
                        PIE_TRACE("[%s] Set white: %f.",
                                  msg->token,
                                  msg->wrkspc->settings.white);
                }
                break;
        case PIE_MSG_SET_BLACK:
                if (msg->f1 < -1.0f || msg->f1 > 1.0f)
                {
                        PIE_WARN("[%s] invalid black: %f.",
                                 msg->token,
                                 msg->f1);
                        ok = -1;
                }
                else
                {
                        msg->wrkspc->settings.black = msg->f1;
                        PIE_TRACE("[%s] Set black: %f.",
                                  msg->token,
                                  msg->wrkspc->settings.black);
                }
                break;
        case PIE_MSG_SET_CLARITY:
                if (msg->f1 < -1.0f || msg->f1 > 1.0f)
                {
                        PIE_WARN("[%s] invalid clarity: %f.",
                                 msg->token,
                                 msg->f1);
                        ok = -1;
                }
                else
                {
                        msg->wrkspc->settings.clarity.amount = msg->f1;
                        PIE_TRACE("[%s] Set clarity: %f.",
                                  msg->token,
                                  msg->wrkspc->settings.clarity.amount);
                }
                break;
        case PIE_MSG_SET_VIBRANCE:
                if (msg->f1 < -1.0f || msg->f1 > 1.0f)
                {
                        PIE_WARN("[%s] invalid vibrance: %f.",
                                 msg->token,
                                 msg->f1);
                        ok = -1;
                }
                else
                {
                        msg->wrkspc->settings.vibrance = msg->f1;
                        PIE_TRACE("[%s] Set vibrance: %f.",
                                  msg->token,
                                  msg->wrkspc->settings.vibrance);
                }
                break;
        case PIE_MSG_SET_SATURATION:
                if (msg->f1 < 0.0f || msg->f1 > 2.0f)
                {
                        PIE_WARN("[%s] invalid saturation: %f.",
                                 msg->token,
                                 msg->f1);
                        ok = -1;
                }
                else
                {
                        msg->wrkspc->settings.saturation = msg->f1;
                        PIE_TRACE("[%s] Set saturation: %f.",
                                  msg->token,
                                  msg->wrkspc->settings.saturation);
                }
                break;
        case PIE_MSG_SET_ROTATE:
                PIE_WARN("[%s] Not implemented yet %d.",
                         msg->token,
                         (int)msg->type);
                ok = -1;
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
                        ok = -1;
                }
                else
                {
                        msg->wrkspc->settings.sharpening.amount = msg->f1;
                        msg->wrkspc->settings.sharpening.radius = msg->f2;
                        msg->wrkspc->settings.sharpening.threshold = msg->f3;
                        PIE_TRACE("[%s] Set sharpening: %f %f %f.",
                                  msg->token,
                                  msg->wrkspc->settings.sharpening.amount,
                                  msg->wrkspc->settings.sharpening.radius,
                                  msg->wrkspc->settings.sharpening.threshold);
                }
                break;
        case PIE_MSG_SET_CURVE:
                PIE_TRACE("Set curve: (%d) %s", msg->i1, msg->buf);

                switch ((enum pie_channel)msg->i1)
                {
                case PIE_CHANNEL_RGB:
                        ok = parse_curve_pnts(&msg->wrkspc->settings.curve_l,
                                              msg->buf);
                        break;
                case PIE_CHANNEL_RED:
                        ok = parse_curve_pnts(&msg->wrkspc->settings.curve_r,
                                              msg->buf);
                        break;
                case PIE_CHANNEL_GREEN:
                        ok = parse_curve_pnts(&msg->wrkspc->settings.curve_g,
                                              msg->buf);
                        break;
                case PIE_CHANNEL_BLUE:
                        ok = parse_curve_pnts(&msg->wrkspc->settings.curve_b,
                                              msg->buf);
                        break;
                default:
                        PIE_WARN("[%s] Invalid channel: %d.",
                                 msg->token,
                                 msg->i1);
                        ok = -1;
                }
                break;
        default:
                PIE_WARN("[%s] Invalid message: %d.",
                         msg->token,
                         (int)msg->type);
                ok = -1;
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
         */
        if (ok == 0)
        {
                struct timing t1;
                struct pie_bitmap_f32rgb* org = &msg->wrkspc->proxy;
                struct pie_bitmap_f32rgb* new = &msg->wrkspc->proxy_out;
                size_t len = org->height * org->row_stride * sizeof(float);
                int r_ok;

                /* Copy fresh proxy */
                timing_start(&t1);
                memcpy(new->c_red, org->c_red, len);
                memcpy(new->c_green, org->c_green, len);
                memcpy(new->c_blue, org->c_blue, len);
                PIE_DEBUG(" Reset proxy:           %8ldusec",
                          timing_dur_usec(&t1));

                r_ok = pie_dev_render(new,
                                      NULL,
                                      &msg->wrkspc->settings);
                assert(r_ok == 0);

                ret_msg = PIE_MSG_RENDER_DONE;
        }

        return ret_msg;
}

static void store_settings(pie_id mob_id,
                           const struct pie_dev_settings* settings)
{
        struct pie_mq_upd_media msg;
        struct pie_dev_settings copy = *settings;
        size_t bw;

        /* Convert to canonical format */
        pie_dev_set_to_can_fmt(&copy);

        msg.type = PIE_MQ_UPD_MEDIA_SETTINGS;
        PIE_DEBUG("Update settings for %lu", mob_id);
        msg.id = pie_htonll(mob_id);
        bw = pie_enc_json_settings(msg.msg,
                                   PIE_MQ_MAX_UPD,
                                   &copy);
        PIE_DEBUG("Update message size: %ld", bw);
        /* bw does not include null terminator */
        if (bw == 0 || (bw + 1)> PIE_MQ_MAX_UPD)
        {
                PIE_ERR("Failed to JSON encode development settings");
                return;
        }

        bw = media_q->send(media_q->this,
                           (char*)&msg,
                           sizeof(msg));
        if (bw != sizeof(msg))
        {
                PIE_ERR("Failed to store new development settings");
        }
}

static void create_proxy(struct pie_bitmap_f32rgb* tgt,
                         struct pie_bitmap_f32rgb* src,
                         int w,
                         int h)
{
        struct timing t;
        struct pie_unsharp_param up = {
                .amount = 0.5f,
                .radius = 0.5f,
                .threshold = 2.0f
        };
        int res;

        timing_start(&t);
        res = pie_bm_dwn_smpl(tgt, src, w, h);
        if (res)
        {
                abort();
        }
        PIE_DEBUG("Downsampled proxy in %ldusec",
                  timing_dur_usec(&t));

        /* Add some sharpening to mitigate any blur created
           during downsampling. */
        timing_start(&t);
        res = pie_alg_unsharp(tgt->c_red,
                              tgt->c_green,
                              tgt->c_blue,
                              &up,
                              tgt->width,
                              tgt->height,
                              tgt->row_stride);
        if (res)
        {
                abort();
        }
        PIE_DEBUG("Added post-downsampling sharpening in %ldusec",
                  timing_dur_usec(&t));
}

static int parse_curve_pnts(struct pie_curve* c, char* msg)
{
        char* lasts;
        char* t;
        int cnt = 0;
        int ret = 0;

        t = strtok_r(msg, ";", &lasts);
        while (t)
        {
                char* p;
                long v;

                PIE_TRACE("Point: %d '%s'", cnt, t);
                p = strchr(t, ',');
                if (p == NULL)
                {
                        PIE_WARN("Invalid coordinate tuple");
                        ret = 1;
                        break;
                }
                /* x */
                v = strtol(t, &p, 10);
                if (p == t)
                {
                        PIE_WARN("Invalid x coordinate");
                        ret = 1;
                        break;
                }
                c->cntl_p[cnt].x = (float)v/1000.0f;

                /* y */
                t = p + 1;
                v = strtol(t, &p, 10);
                if (p == t)
                {
                        PIE_WARN("Invalid y coordinate");
                        ret = 1;
                        break;
                }
                c->cntl_p[cnt].y = (float)v/1000.0f;

                cnt++;
                t = strtok_r(NULL, ";", &lasts);
        };
        c->num_p = cnt;

        for (int i = 0; i < c->num_p; i++)
        {
                PIE_TRACE("%f: %f", c->cntl_p[i].x, c->cntl_p[i].y);
        }

        return ret;
}
