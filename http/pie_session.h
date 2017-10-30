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

#ifndef __PIE_SESSION_H__
#define __PIE_SESSION_H__

#define PIE_SESS_TOKEN_LEN 41 /* Token is 40 chars, add one for null */
#define PIE_TX_IMG  0x1

#include "../lib/timing.h"

struct pie_sess_mgr;
struct pie_img_workspace;

/**
 * Represents a session.
 * All data kept in the session must be managed by a third party.
 * The session manager is free to de-alocate a session at any time,
 * and hence the data referenced by the session must be managed by
 * another entity to avoid memory leaks.
 */
struct pie_sess
{
        char token[PIE_SESS_TOKEN_LEN];
        struct pie_img_workspace* wrkspc;
        unsigned char* rgba;
        /* Used for duration measurement, set by cb_cmd */
        struct timing t;
        /* Timestamp for last accessed time, in seconds */
        long access_ts;
        int rgba_len;
        /* New data can be written to client */
        unsigned char tx_ready;
};

/**
 * Initialize a session.
 * During initialization the token is generated.
 * @param void
 * @return a new session.
 */
extern struct pie_sess* pie_sess_create(void);

/**
 * Destroy a session.
 * @param the session to destroy.
 * @return void.
 */
extern void pie_sess_destroy(struct pie_sess* s);

/**
 * Create a session manager.
 * @param void.
 * @return a session manager.
 */
extern struct pie_sess_mgr* pie_sess_mgr_create(void);

/**
 * Destroy a session manager.
 * Any sessions stored will be freed/destroyed.
 * @param the session manager to destroy.
 * @return void.
 */
extern void pie_sess_mgr_destroy(struct pie_sess_mgr*);

/**
 * Retrieve a session.
 * When a valid session is found, the access timestamp is updated
 * before it is returned.
 * @param session manager.
 * @param the token to search for.
 * @return pointer to the session or NULL if not found.
 */
extern struct pie_sess* pie_sess_mgr_get(struct pie_sess_mgr*, char*);

/**
 * Store a session.
 * The session must be initialized before it can be sored.
 * Before the session is stored, the access timestamp is updated.
 * @param session manager.
 * @param session to store.
 * @return void.
 */
extern void pie_sess_mgr_put(struct pie_sess_mgr*, struct pie_sess*);

/**
 * Look for sessions that are older than a specific duration.
 * @param session manager.
 * @param the threshold in seconds.
 * @return number of sessions that were repaed.
 */
extern int pie_sess_mgr_reap(struct pie_sess_mgr*, long);

#endif /* __PIE_SESSION_H__ */
