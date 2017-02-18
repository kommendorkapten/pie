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

#include <stddef.h>

#define MAX_HEADERS 4096

struct lws;
struct hmap;
struct pie_sess;
struct pie_sess_mgr;

/**
 * Return the mime type from the requested URL.
 * @param the request URL.
 * @return the mimetype (e.g image/jpeg) or NULL if unknown.
 */
extern const char* get_mimetype(const char*);

/**
 * Exract request headers from the current request.
 * Limitations: Only a signel value per key can be stored.
 * @param the request
 * @return pointer to a hash map of the headers.
 */
extern struct hmap* get_request_headers(struct lws*);

/**
 * Extract a session from the current request.
 * A session is defined by the cookie pie-session.
 * @param the session manager that owns the sessions.
 * @param the request
 * @return pointer to the session, or NULL if no session is found.
 */
extern struct pie_sess* get_session(struct pie_sess_mgr*, struct lws*);

/**
 * Get a cookie from a request.
 * @param pointer on where to store the value.
 * @param lws request object.
 * @param cookie to search for.
 * @param max length of value.
 * @return 0 if the cookie was found. Non zero otherwise.
 */
extern int get_lws_cookie(char* restrict,
                          struct lws*,
                          const char* restrict,
                          size_t);
