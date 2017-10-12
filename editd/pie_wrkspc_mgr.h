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

#ifndef __PIE_WRKSPC_MGR_H__
#define __PIE_WRKSPC_MGR_H__

#include <sqlite3.h>
#include "../pie_types.h"
#include "../pie_id.h"

struct pie_wrkspc_mgr;

/* The main workspace for an image being open in editd. */
struct pie_img_workspace
{
        pie_id mob_id;
        struct pie_histogram hist;
        struct pie_dev_settings settings;
        /* Unmodified full resolution image */
        struct pie_bitmap_f32rgb raw;
        /* Downsampled unmodified proxy image */
        struct pie_bitmap_f32rgb proxy;
        /* Downsampled and rendered proxy image */
        struct pie_bitmap_f32rgb proxy_out;
};

/**
 * Create an image workspace manager.
 * The manager creates (loads images from disk) and prepares a workspace.
 * @param database to use
 * @param cache size for workspaces.
 * @return a work space manager.
 */
extern struct pie_wrkspc_mgr* pie_wrkspc_mgr_create(sqlite3*, int);

/**
 * Load an image from disk. Proxy, and proxy_out are not allocated.
 * @param the mangaer.
 * @param mob to load.
 * @return an image workspace, or NULL if cache is full.
 */
extern struct pie_img_workspace* pie_wrkspc_mgr_acquire(struct pie_wrkspc_mgr*,
                                                        pie_id);

/**
 * Return an image workspace.
 * @param the manager.
 * @param the workspace to release.
 * @return void.
 */
extern void pie_wrkspc_mgr_release(struct pie_wrkspc_mgr*,
                                   struct pie_img_workspace*);

/**
 * Destroy a manager. This will free any created workspaces.
 * @param manager to destroy.
 * @return void
 */
extern void pie_wrkspc_mgr_destroy(struct pie_wrkspc_mgr*);

#endif /* __PIE_WRKSPC_MGR_H__ */

