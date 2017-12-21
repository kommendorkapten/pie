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

#include <unistd.h>
#include "pie_doml_mob.h"
#include "../pie_types.h"
#include "../pie_log.h"
#include "../dm/pie_min.h"
#include "../dm/pie_mob.h"
#include "../dm/pie_exif_data.h"
#include "../dm/pie_dev_params.h"
#include "../dm/pie_collection_member.h"
#include "../lib/llist.h"
#include "../lib/fal.h"
#include "../cfg/pie_cfg.h"

static int pie_doml_unlink(const char*, int);

int pie_doml_mob_delete(sqlite3* db, pie_id mob_id)
{
        char path[PIE_PATH_LEN];
        struct pie_stg_mnt_arr* stgs = NULL;
        struct llist* l = NULL;
        struct lnode* n;
        int ret = -1;
        int proxy = -1;
        int thumb = -1;

        PIE_DEBUG("Mob: %ld", mob_id);
        stgs = pie_cfg_get_hoststg(-1);
        for (int i = 0; i < stgs->len; i++)
        {
                if (stgs->arr[i])
                {
                        switch (stgs->arr[i]->stg.stg_type)
                        {
                        case PIE_STG_THUMB:
                                thumb = i;
                                break;
                        case PIE_STG_PROXY:
                                proxy = i;
                                break;
                        }
                }
        }
        if (proxy < 0 || thumb < 0)
        {
                PIE_WARN("Proxy or thumb is not mounted");
                goto done;
        }

        /* Find all MINs and their storages, unlink files */
        l = pie_min_find_mob(db, mob_id);

        if (l == NULL)
        {
                PIE_WARN("Error fetching MINs for MOB %ld", mob_id);
                goto done;
        }

        /* Verify all MINs are on storages mounted by this host */
        for (n = llist_head(l); n != NULL; n = n->next)
        {
                struct pie_min* min = n->data;

                if (min->min_stg_id > stgs->len ||
                    stgs->arr[min->min_stg_id] == NULL)
                {
                        PIE_WARN("Storage %d is not mounted", min->min_stg_id);
                        goto done;
                }
        }

        /* unlink files */
        snprintf(path,
                 PIE_PATH_LEN,
                 "%s/%ld.jpg",
                 stgs->arr[proxy]->mnt_path,
                 mob_id);
        pie_doml_unlink(path, PIE_STG_PROXY);
        snprintf(path,
                 PIE_PATH_LEN,
                 "%s/%ld.jpg",
                 stgs->arr[thumb]->mnt_path,
                 mob_id);
        pie_doml_unlink(path, PIE_STG_THUMB);
        for (n = llist_head(l); n != NULL; n = n->next)
        {
                struct pie_min* min = n->data;
                int type = stgs->arr[min->min_stg_id]->stg.stg_type;

                PIE_DEBUG("Remove MIN %ld at storage %d",
                          min->min_id,
                          min->min_stg_id);

                snprintf(path,
                         PIE_PATH_LEN,
                         "%s%s",
                         stgs->arr[min->min_stg_id]->mnt_path,
                         min->min_path);
                pie_doml_unlink(path, type);

                /* Delete MIN from database */
                pie_min_delete(db, min);
        }

        /* Delete any EXIF data */
        struct pie_exif_data ped = {.ped_mob_id = mob_id};
        pie_exif_data_delete(db, &ped);

        /* Delete any development parameters */
        struct pie_dev_params pdp = {.pdp_mob_id = mob_id};
        pie_dev_params_delete(db, &pdp);

        /* Delete collection membership */
        pie_collection_member_delete_mob(db, mob_id);

        /* Delete MOB from database */
        struct pie_mob mob = {.mob_id = mob_id};
        pie_mob_delete(db, &mob);

        ret = 0;
done:
        if (stgs)
        {
                pie_cfg_free_hoststg(stgs);
        }
        if (l)
        {
                for (n = llist_head(l); n != NULL; n = n->next)
                {
                        pie_min_free(n->data);
                }
                llist_destroy(l);
        }

        return ret;
}

static int pie_doml_unlink(const char* path, int stg_type)
{
        int valid_stg = 0;

        switch (stg_type)
        {
        case PIE_STG_ONLINE:
        case PIE_STG_THUMB:
        case PIE_STG_PROXY:
                valid_stg = 1;
        }

        if (!valid_stg)
        {
                PIE_ERR("Unsupported storage type: %d", stg_type);
                return -1;
        }

        PIE_DEBUG("Remove '%s' on storage type %d\n", path, stg_type);

        return unlink(path);
}
