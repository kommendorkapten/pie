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

#ifndef __PIE_UTIL_H__
#define __PIE_UTIL_H__

#include <stddef.h>

#define MAX_HEADERS 4096

struct lws;
struct hmap;
struct pie_http_sess;
struct pie_http_sess_mgr;

enum pie_http_verb
{
        PIE_HTTP_VERB_UNKNOWN,
        PIE_HTTP_VERB_GET,
        PIE_HTTP_VERB_PUT,
        PIE_HTTP_VERB_POST,
        PIE_HTTP_VERB_DELETE
};

struct pie_http_post_data
{
        char* data;
        size_t p;
        size_t cap;
};

/**
 * Return the mime type from the requested URL.
 * @param the request URL.
 * @return the mimetype (e.g image/jpeg) or NULL if unknown.
 */
extern const char* pie_http_get_mimetype(const char*);

/**
 * Exract request query parameters.
 * Limitations: Only a signel value per key can be stored.
 * @param the request
 * @return pointer to a hash map of the query parameters.
 */
extern struct hmap* pie_http_req_params(struct lws*);

/**
 * Extract a session from the current request.
 * A session is defined by the cookie pie-session.
 * @param the session manager that owns the sessions.
 * @param the request
 * @return pointer to the session, or NULL if no session is found.
 */
extern struct pie_http_sess* pie_http_get_session(struct pie_http_sess_mgr*,
                                                  struct lws*);

/**
 * Get a cookie from a request.
 * @param pointer on where to store the value.
 * @param lws request object.
 * @param cookie to search for.
 * @param max length of value.
 * @return 0 if the cookie was found. Non zero otherwise.
 */
extern int pie_http_get_cookie(char* restrict,
                               struct lws*,
                               const char* restrict,
                               size_t);

/**
 * Write data to lws request object.
 * Status code will be set to 200, no extra headers
 * will be added (except content type).
 * @param lws request object.
 * @param content type.
 * @param data to write.
 * @param number of bytes to write.
 * @param content type.
 */
extern ssize_t pie_http_lws_write(struct lws*,
                                  unsigned char* restrict,
                                  size_t,
                                  const char* restrict);

/**
 * Get the HTTP verb in the current transcation.
 * @param lws request object.
 * @return the HTTP verb.
 */
extern enum pie_http_verb pie_http_verb_get(struct lws*);

/**
 * Get a string representation of a HTTP verb.
 * @param http verb.
 * @return human readable representation of the HTTP verb.
 */
extern const char* pie_http_verb_string(enum pie_http_verb);

/**
 * Init post data struct.
 * @param post data struct.
 * @param initial capacity.
 * @return 0 on success.
 */
extern int pie_http_post_data_init(struct pie_http_post_data*, size_t);

/**
 * Add data.
 * @param post data struct.
 * @param data.
 * @param bytes to add.
 * @return 0 on success.
 */
extern int pie_http_post_data_add(struct pie_http_post_data*,
                                  const void*,
                                  size_t);
#endif /* __PIE_UTIL_H__ */
