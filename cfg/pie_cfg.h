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

#ifndef __PIE_CFG_H__
#define __PIE_CFG_H__

#include <sqlite3.h>
#include "../pie_types.h"
#include "../dm/pie_storage.h"
#define PIE_CFG_PATH "/etc/pie.conf"

/*
 * A static config interface.
 * Sources for config are main config file (default is /etc/pie.conf),
 * and the main database.
 */

struct pie_host;

/*
 * Extends a storage with a mount point.
 */
struct pie_stg_mnt
{
        struct pie_storage stg;
        char mnt_path[PIE_PATH_LEN];
};

/*
 * Utility struct to allow O(1) access to storages based on their id.
 * Storages are stored in arr with offset of their storage id,
 * slots not occupied are NULL. Len contains the number of entries
 * the array.
 */
struct pie_stg_mnt_arr
{
        struct pie_stg_mnt** arr;
        int len;
};

/**
 * Initialize the config module.
 * @param path to configuration file to load.
 * @param 0 on success.
 */
extern int pie_cfg_load(const char*);

/**
 * Test if configuration has been loaded.
 * @param void.
 * @return 1 if config was successfully loaded.
 */
extern int pie_cfg_loaded(void);

/**
 * Destroy and free any allocated resources maintained by this module.
 * @param void
 * @return void
 */
extern void pie_cfg_close(void);

/**
 * Return a handle to the database;
 * @param void
 * @return Pointer to a database handle, or NULL if none available.
 */
extern sqlite3* pie_cfg_get_db(void);

/**
 * Get a value from the config file.
 * @param key to get value for.
 * @return null terminated string with value, or NULL if none found.
 */
extern const char* pie_cfg_get(const char*);

/**
 * Read a value from the config file and interpret it like an integer.
 * @param key to get value from.
 * @param pointer where to read data into.
 * @return 0 if the value existed and could be interpreted as an integer,
 *         non zero otherwise.
 */
extern int pie_cfg_get_long(const char*, long*);

/**
 * Get a host entry from a host id.
 * If current host is to be resolved, the unqualified hostname is use to
 * find an entry in the pie_host table.
 * @param host id to lookup, or -1 if current host should be used.
 * @return a pie_host entry or NULL.
 */
extern struct pie_host* pie_cfg_get_host(int);

/**
 * Get a host entry from unqualified hostname.
 * @param hostname to look up.
 * @return a pie_host entry or NULL.
 */
extern struct pie_host* pie_cfg_get_hostbyname(const char*);

/**
 * Get all storages for a specific host.
 * @param the host id
 * @return a pointer to struct pie_sgt_mnt_arr, or NULL
 *         if a failure occurs.
 */
extern struct pie_stg_mnt_arr* pie_cfg_get_hoststg(int);

/**
 * Destroy a pie_stg_mnt_arr
 * @param struct to free.
 * @return void.
 */
extern void pie_cfg_free_hoststg(struct pie_stg_mnt_arr*);

#define PIE_CFG_GET_STORAGE(arr_ptr, stg_id) ((stg_id) >= (arr_ptr)->len ? NULL : (arr_ptr)->arr[(stg_id)])

#endif /* __PIE_CFG_H__ */
