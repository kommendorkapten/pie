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

#ifndef __PIE_MSG_H__
#define __PIE_MSG_H__

#include "../lib/timing.h"

struct pie_editd_workspace;

/**
 * Pie message semantics over channels are:
 * Sender allocates memory for message.
 * Receiver frees memory after use.
 * This semantic is NOT compatible with fan in/out.
 */

#define PIE_MSG_BUF_LEN 256
#define PIE_MSG_TOKEN_LEN 41

enum pie_msg_type
{
        PIE_MSG_INVALID,
        PIE_MSG_LOAD,
        PIE_MSG_LOAD_DONE,
        PIE_MSG_VIEWPORT,
        PIE_MSG_RENDER_DONE,
        PIE_MSG_NEW_PROXY_DIM,
        PIE_MSG_SET_COLOR_TEMP,
        PIE_MSG_SET_TINT,
        PIE_MSG_SET_EXSPOSURE,
        PIE_MSG_SET_CONTRAST,
        PIE_MSG_SET_HIGHL,
        PIE_MSG_SET_SHADOW,
        PIE_MSG_SET_WHITE,
        PIE_MSG_SET_BLACK,
        PIE_MSG_SET_CLARITY,
        PIE_MSG_SET_VIBRANCE,
        PIE_MSG_SET_SATURATION,
        PIE_MSG_SET_ROTATE,
        PIE_MSG_SET_SHARP,
        PIE_MSG_SET_CURVE,

        /* Always last */
        PIE_MSG_COUNT
};

struct pie_msg
{
        enum pie_msg_type type;
        char buf[PIE_MSG_BUF_LEN];
        /* A general token, can contain a null terminated,
           hex encoded sha1 sum */
        char token[PIE_MSG_TOKEN_LEN];
        struct pie_editd_workspace* wrkspc;
        struct timing t;
        float f1;
        float f2;
        float f3;
        int i1;
        int i2;
        int i3;
        int i4;
        int i5;
        int i6;
};

/**
 * Allocate and initialize a message.
 * Default values are 0 and MSG_TYPE_INVALID, token is the empty string.
 * @param void.
 * @return a message.
 */
extern struct pie_msg* pie_msg_alloc(void);

/**
 * Frees a message.
 * @param message to free.
 * @return void.
 */
extern void pie_msg_free(struct pie_msg*);

#endif /* __PIE_MSG_H__ */
