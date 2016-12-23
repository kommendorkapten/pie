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
#include "../pie_log.h"
#include "../pie_render.h"
#include "../io/pie_io.h"
#include "../encoding/pie_json.h"
#include "../alg/pie_hist.h"
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <netinet/in.h>
#ifdef _HAS_SSE
# include <nmmintrin.h> /* sse 4.2 */
#endif
#include <note.h>

#define _USE_GAMMA_CONV 0
#define JSON_HIST_SIZE (10 * 1024)

struct pie_server server;

struct config
{
        char* lib_path;
};
static struct config config;

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
 * Callback. Update an image and renders the image.
 * @param message to acton.
 * @return if the message should be retransmit, the new msg type,
 *         if message should be dropped, PIE_MSG_INVALID.
 */
static enum pie_msg_type cb_msg_render(struct pie_msg*);

/**
 * Encode the proxy_out into rgba format.
 * @param the image workspace to encode.
 * @return void
 */
static void encode_rgba(struct pie_img_workspace*);

int main(void)
{
        pthread_t thr_ev_loop;
        pthread_t thr_int;
        sigset_t b_sigset;
        void* ret;
        int ok;

        config.lib_path = "test-images";
        server.run = 1;
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

        PIE_LOG("Start with config:");
        PIE_LOG("  image library path: %s", config.lib_path);
        
        if (start_server(&server))
        {
                PIE_ERR("Failed");
        }

        
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

        return 0;
}

static void sig_h(int signum)
{
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
        PIE_DEBUG("Leaving.");
        s->run = 0;

        return ret;
}

static void* ev_loop(void* a)
{
        struct chan_msg msg;
        struct pie_server* s = (struct pie_server*)a;
        void* ret = NULL;

        PIE_DEBUG("Ready for messages.");

        while (s->run)
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
        char buf[PIE_PATH_LEN];
        struct timing t;
        int stride;
        int len;
        int res;
        int w;
        int h;

        /* HACK */
        if (msg->img)
        {
                return PIE_MSG_LOAD_DONE;
        }

        assert(msg->img == NULL);

        msg->img = malloc(sizeof(struct pie_img_workspace));
        memset(msg->img, 0, sizeof(struct pie_img_workspace));
        snprintf(buf, PIE_PATH_LEN, "%s/%s", config.lib_path, msg->buf);
        timing_start(&t);
        res = pie_io_load(&msg->img->raw, buf);
        PIE_DEBUG("Loaded %s %ldusec", buf, timing_dur_usec(&t));        
        if (res)
        {
                PIE_ERR("[%s] Could not open '%s'",
                        msg->token,
                        buf);
                free(msg->img);
                return PIE_MSG_INVALID;
        }
        stride = msg->img->raw.row_stride;
#if _USE_GAMMA_CONV > 0
        for (int y = 0; y < msg->img->raw.height; y++)
        {
                srgb_to_linearv(msg->img->raw.c_red + y * stride, 
                                msg->img->raw.width);
                srgb_to_linearv(msg->img->raw.c_green + y * stride, 
                                msg->img->raw.width);
                srgb_to_linearv(msg->img->raw.c_blue + y * stride, 
                                msg->img->raw.width);
                                
        }
#endif        
        w = msg->i1;
        h = msg->i2;
        w = w < msg->img->raw.width ? w : msg->img->raw.width;
        h = h < msg->img->raw.height ? h : msg->img->raw.height;

        PIE_DEBUG("[%s] Load proxy with size %dx%d", msg->token, w, h);
        strncpy(msg->img->path, buf, PIE_PATH_LEN);
        /* Read from database to get settings */
        pie_img_init_settings(&msg->img->settings);
        /* Allocate proxy images */
        msg->img->proxy.width = w;
        msg->img->proxy.height = h;
        msg->img->proxy.color_type = PIE_COLOR_TYPE_RGB;
        bm_alloc_f32(&msg->img->proxy);
        msg->img->proxy_out.width = w;
        msg->img->proxy_out.height = h;
        msg->img->proxy_out.color_type = PIE_COLOR_TYPE_RGB;
        bm_alloc_f32(&msg->img->proxy_out);

        /* Allocate output buffer */
        len = (int)(w * h * 4 + 2 * sizeof(uint32_t));
        msg->img->buf_proxy = malloc(len + PROXY_RGBA_OFF);
        msg->img->proxy_out_rgba = msg->img->buf_proxy + PROXY_RGBA_OFF;
        msg->img->proxy_out_len = len;

        /* Allocate JSON output buffer */
        msg->img->buf_hist = malloc(JSON_HIST_SIZE + PROXY_RGBA_OFF);
        msg->img->hist_json = msg->img->buf_hist + PROXY_RGBA_OFF;

        /* Call pie_downsample */
        len = (int)(h * stride * sizeof(float));
        /* Copy raw to proxy */
        memcpy(msg->img->proxy.c_red, msg->img->raw.c_red, len);
        memcpy(msg->img->proxy.c_green, msg->img->raw.c_green, len);
        memcpy(msg->img->proxy.c_blue, msg->img->raw.c_blue, len);

        /* Copy proxy to proxy out */
        memcpy(msg->img->proxy_out.c_red, msg->img->proxy.c_red, len);
        memcpy(msg->img->proxy_out.c_green, msg->img->proxy.c_green, len);
        memcpy(msg->img->proxy_out.c_blue, msg->img->proxy.c_blue, len);

        /* Encode proxy rgba */
        encode_rgba(msg->img);

        /* Create histogram */
        pie_alg_hist_lum(&msg->img->hist, &msg->img->proxy_out);
        pie_alg_hist_rgb(&msg->img->hist, &msg->img->proxy_out);
        msg->img->hist_json_len = pie_json_enc_hist((char*)msg->img->hist_json, 
                                                    JSON_HIST_SIZE, 
                                                    &msg->img->hist);
        /* Issue a load cmd */

        return PIE_MSG_LOAD_DONE;
}

static enum pie_msg_type cb_msg_render(struct pie_msg* msg)
{
        int status = 0;
#if 0
        int resample = 0;
#endif        
        switch (msg->type)
        {
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
                }
                break;
        case PIE_MSG_SET_ESPOSURE:
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
                }
                break;
        case PIE_MSG_SET_HIGHL:
                PIE_WARN("[%s] Not implemented yet %d.",
                         msg->token,
                         (int)msg->type);
                status = -1;
                break;
        case PIE_MSG_SET_SHADOW:
                PIE_WARN("[%s] Not implemented yet %d.",
                         msg->token,
                         (int)msg->type);
                status = -1;
                break;
        case PIE_MSG_SET_WHITE:
                PIE_WARN("[%s] Not implemented yet %d.",
                         msg->token,
                         (int)msg->type);
                status = -1;
                break;
        case PIE_MSG_SET_BLACK:
                PIE_WARN("[%s] Not implemented yet %d.",
                         msg->token,
                         (int)msg->type);
                status = -1;
                break;
        case PIE_MSG_SET_CLARITY:
                PIE_WARN("[%s] Not implemented yet %d.",
                         msg->token,
                         (int)msg->type);
                status = -1;
                break;
        case PIE_MSG_SET_VIBRANCE:
                PIE_WARN("[%s] Not implemented yet %d.",
                         msg->token,
                         (int)msg->type);
                status = -1;
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
                }
                break;
        case PIE_MSG_SET_ROTATE:
                PIE_WARN("[%s] Not implemented yet %d.",
                         msg->token,
                         (int)msg->type);
                status = -1;
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
        if (status == 0)
        {
                struct timing t1;
                struct bitmap_f32rgb* org = &msg->img->proxy;
                struct bitmap_f32rgb* new = &msg->img->proxy_out;
                size_t len = org->height * org->row_stride * sizeof(float);
                int r_ok;
                
                /* Copy fresh proxy */
                timing_start(&t1);
                memcpy(new->c_red, org->c_red, len);
                memcpy(new->c_green, org->c_green, len);
                memcpy(new->c_blue, org->c_blue, len);
                PIE_DEBUG(" Reset proxy:           %ldusec",
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
                PIE_DEBUG(" Created histogram:     %ldusec",
                          timing_dur_usec(&t1));
                
                /* Write proxy to RGBA */
                timing_start(&t1);
                encode_rgba(msg->img);
                PIE_DEBUG(" Encoded proxy:         %ldusec",
                          timing_dur_usec(&t1));
                timing_start(&t1);
                /* Create JSON for hist */
                msg->img->hist_json_len = pie_json_enc_hist((char*)msg->img->hist_json, 
                                                            JSON_HIST_SIZE, 
                                                            &msg->img->hist);
                PIE_DEBUG("JSON encoded histogram: %ldusec",
                          timing_dur_usec(&t1));

                return PIE_MSG_RENDER_DONE;
        }

        return PIE_MSG_INVALID;
}

#ifdef _HAS_SIMD
# ifdef _HAS_SSE

static void encode_rgba(struct pie_img_workspace* img)
{
        __m128 coeff_scale = _mm_set1_ps(255.0f);
        unsigned char* p = img->proxy_out_rgba;
        int stride = img->raw.row_stride;
        int rem = img->proxy_out.width % 4;
        int stop = img->proxy_out.width - rem;
        uint32_t w = htonl(img->proxy_out.width);
        uint32_t h = htonl(img->proxy_out.height);
        float or[4];
        float og[4];
        float ob[4];

        memcpy(p, &w, sizeof(uint32_t));
        p += sizeof(uint32_t);
        memcpy(p, &h, sizeof(uint32_t));
        p += sizeof(uint32_t);

        for (int y = 0; y < img->proxy_out.height; y++)
        {
                for (int x = 0; x < stop; x += 4)
                {
                        int i = y * stride + x;
                        __m128 r = _mm_load_ps(&img->proxy_out.c_red[i]);
                        __m128 g = _mm_load_ps(&img->proxy_out.c_green[i]);
                        __m128 b = _mm_load_ps(&img->proxy_out.c_blue[i]);

                        r = _mm_mul_ps(r, coeff_scale);
                        g = _mm_mul_ps(g, coeff_scale);
                        b = _mm_mul_ps(b, coeff_scale);

                        _mm_store_ps(or, r);
                        _mm_store_ps(og, g);
                        _mm_store_ps(ob, b);

                        *p++ = (unsigned char)or[0];
                        *p++ = (unsigned char)og[0];
                        *p++ = (unsigned char)ob[0];
                        *p++ = 255;
                        *p++ = (unsigned char)or[1];
                        *p++ = (unsigned char)og[1];
                        *p++ = (unsigned char)ob[1];
                        *p++ = 255;
                        *p++ = (unsigned char)or[2];
                        *p++ = (unsigned char)og[2];
                        *p++ = (unsigned char)ob[2];
                        *p++ = 255;
                        *p++ = (unsigned char)or[3];
                        *p++ = (unsigned char)og[3];
                        *p++ = (unsigned char)ob[3];
                        *p++ = 255;
                }

                for (int x = stop; x < img->proxy_out.width; x++)
                {
                        int i = y * stride + x;
                        unsigned char r, g, b;
                        r = (unsigned char)(img->proxy_out.c_red[i] * 255.0f);
                        g = (unsigned char)(img->proxy_out.c_green[i] * 255.0f);
                        b = (unsigned char)(img->proxy_out.c_blue[i] * 255.0f);
                        
                        *p++ = r;
                        *p++ = g;
                        *p++ = b;
                        *p++ = 255;
                }
        }        
        
}

# elif _HAS_ALTIVEC
#  error ALTIVET NOT IMPLEMENTED
# endif

#else

static void encode_rgba(struct pie_img_workspace* img)
{
        unsigned char* p = img->proxy_out_rgba;
        int stride = img->raw.row_stride;
        uint32_t w = htonl(img->proxy_out.width);
        uint32_t h = htonl(img->proxy_out.height);

        memcpy(p, &w, sizeof(uint32_t));
        p += sizeof(uint32_t);
        memcpy(p, &h, sizeof(uint32_t));
        p += sizeof(uint32_t);

        for (int y = 0; y < img->proxy_out.height; y++)
        {
#if _USE_GAMMA_CONV > 0
                /* Convert to sRGB */
                linear_to_srgbv(img->proxy_out.c_red + y * stride, 
                                img->proxy_out.width);
                linear_to_srgbv(img->proxy_out.c_green + y * stride, 
                                img->proxy_out.width);
                linear_to_srgbv(img->proxy_out.c_blue + y * stride, 
                                img->proxy_out.width);
#endif
                for (int x = 0; x < img->proxy_out.width; x++)
                {
                        unsigned char r, g, b;
                        
                        r = (unsigned char)(img->proxy_out.c_red[y * stride + x] * 255.0f);
                        g = (unsigned char)(img->proxy_out.c_green[y * stride + x] * 255.0f);
                        b = (unsigned char)(img->proxy_out.c_blue[y * stride + x] * 255.0f);
                        
                        *p++ = r;
                        *p++ = g;
                        *p++ = b;
                        *p++ = 255;
                }
        }        
}

#endif /* _HAS_SIMD */
