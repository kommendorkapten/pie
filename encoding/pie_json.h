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

#ifndef __PIE_JSON_H__
#define __PIE_JSON_H__

#include "../pie_id.h"
#include <stdlib.h>

struct pie_histogram;
struct pie_dev_settings;
struct pie_unsharp_param;
struct pie_exif_data;
struct pie_collection;
struct llist;
struct pie_mob;

/**
 * Encode a histogram to JSON.
 * A maximum sized histogram encodes to ~5Kib.
 * @param the buffer to write JSON to.
 * @param number of bytes the buffer can hold.
 * @param histogram to encode.
 * @return the number of bytes written (excluding the null byte; if
 *         it was written).
 */
extern size_t pie_enc_json_hist(char*, size_t, const struct pie_histogram*);

extern size_t pie_enc_json_settings(char*,
                                    size_t,
                                    const struct pie_dev_settings*);

extern int pie_dec_json_settings(struct pie_dev_settings*, char*);

extern int pie_dec_json_unsharp(struct pie_unsharp_param*, char*);

extern size_t pie_enc_json_exif(char*, size_t, const struct pie_exif_data*);

extern size_t pie_enc_json_collection(char*,
                                      size_t,
                                      const struct pie_collection*,
                                      struct llist*);

extern size_t pie_enc_json_collection_list(char*, size_t, struct llist*);

extern size_t pie_enc_json_mob(char*, size_t, const struct pie_mob*);

extern int pie_dec_json_mob(struct pie_mob*, char*);

#endif /* __PIE_JSON_H__ */
