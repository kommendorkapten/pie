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

#include <stdlib.h>
#include <sqlite3.h>
#include <unistd.h>
#include "pie_doml_stg.h"
#include "../lib/llist.h"
#include "../dm/pie_min.h"
#include "../cfg/pie_cfg.h"
#include "../pie_log.h"

struct pie_min* pie_doml_min_for_mob(sqlite3* db,
                                     struct pie_stg_mnt** stgs,
                                     int len,
                                     pie_id mob_id)
{
        struct llist* l = pie_min_find_mob(db, mob_id);
        struct lnode* c;
        void* min = NULL;

        if (l == NULL)
        {
                return NULL;
        }

        /* Naive implementation. Take the first matching MIN. */
        c = llist_head(l);
        while (c)
        {
                struct pie_min* cand = c->data;

                PIE_DEBUG("Found MIN %ld for MOB %ld", cand->min_id, mob_id);

                for (int i = 0; i < len; i++)
                {
                        const struct pie_stg_mnt* stg = stgs[i];

                        if (stg == NULL)
                        {
                                continue;
                        }

                        if (stg->stg.stg_id == cand->min_stg_id)
                        {
                                min = cand;
                                goto done;
                        }
                }

                c = c->next;
        }

done:
        c = llist_head(l);
        while (c)
        {
                if (c->data != min)
                {
                        free(c->data);
                }

                c = c->next;
        }

        llist_destroy(l);

        return min;
}

int pie_doml_file_exists(int stg, const char* rel_path)
{
        char path[PIE_PATH_LEN];
        struct pie_stg_mnt_arr* stgs;
        int ok;
        int ret = 0;

        stgs = pie_cfg_get_hoststg(-1);
        if (stgs == NULL)
        {
                PIE_ERR("Could not get storages");
                return -1;
        }

        if (stgs->arr[stg] == NULL)
        {
                PIE_WARN("Storage %d is not mounted", stg);
                return -2;
        }

        snprintf(path,
                 PIE_PATH_LEN,
                 "%s%s",
                 stgs->arr[stg]->mnt_path,
                 rel_path);

        PIE_TRACE("Test %s for existance", path);
        ok = access(path, F_OK);

        if (ok == 0)
        {
                /* File exists */
                ret = 1;
        }
        /* If ok != 0, errno should be inspected. Skip for now */

        pie_cfg_free_hoststg(stgs);

        return ret;
}
