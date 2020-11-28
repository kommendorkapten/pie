/*
* Copyright (C) 2017 Fredrik Skogman, skogman - at - gmail.com.
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

#ifndef __PIE_MH_STG_H__
#define __PIE_MH_STG_H__

#include <sqlite3.h>
#include "../prunt/pie_id.h"

struct pie_min;
struct pie_stg_mnt;

/**
 * Return the most suitable MIN for a given MOB and set of storages.
 * @param handle to the database.
 * @param array of pie_stg_mnt available.
 * @param length of array.
 * @param mob id
 * @return a min object, or NULL if no MIN could be find for the
 *         provided host.
 */
extern struct pie_min* pie_mh_min_for_mob(sqlite3*,
                                          struct pie_stg_mnt**,
                                          int,
                                          pie_id);

/**
 * Check if a file exists on a storage. Storage must be mounted on
 * current host.
 * @param storage id.
 * @param relative path on storage.
 * @return 1 if file exists. 0 if file does not exits. Negative on error.
 */
extern int pie_mh_file_exists(int, const char*);

#endif /* __PIE_MH_STG_H__ */
