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

#include <sqlite3.h>
#include <string.h>
#include <unistd.h>
#include "../io/pie_io.h"
#include "../bm/pie_bm.h"
#include "../pie_types.h"
#include "../pie_id.h"
#include "../pie_log.h"
#include "../cfg/pie_cfg.h"
#include "../dm/pie_storage.h"
#include "../dm/pie_host.h"

struct row
{
        pie_id mob_id;
        int stg_id;
        char min_path[PIE_PATH_LEN];
};

static int check_path(const char*);
static int tojpg(const char*, const char*, int);

int main(int argc, char** argv)
{
        struct pie_stg_mnt_arr* storages = NULL;
        struct pie_host* host = NULL;
        struct pie_stg_mnt* stg_proxy = NULL;
        struct pie_stg_mnt* stg_thumb = NULL;
        char* q = "SELECT mob_id, min_stg_id, min_path FROM pie_mob LEFT JOIN pie_min ON mob_id=min_mob_id";
        sqlite3_stmt *pstmt;
        int status = 1;
        int mob_ctr = 0;
        int err_cnt = 0;
        int err_fix = 0;
        int ret;
        int verbose = 0;
        int dry_run = 0;
        int c;
        int max_thumb;
        long lval;

        while ((c = getopt(argc, argv, "vd")) != -1)
        {
                switch (c)
                {
                case 'd':
                        dry_run = 1;
                        PIE_LOG("Entering dry run. No modificatons will be done");
                        break;
                case 'v':
                        verbose = 1;
                        break;
                }
        }

        if (pie_cfg_load(PIE_CFG_PATH))
        {
                PIE_ERR("Failed to read conf");
                goto done;
        }

        if (pie_cfg_get_long("image:thumbnail:size", &lval))
        {
                PIE_WARN("Config image:thumbnail:size not found in config");
                lval = 256;
        }
        else
        {
                max_thumb = (int)lval;
        }

        if (host = pie_cfg_get_host(-1), host == NULL)
        {
                PIE_ERR("Failed to get host");
                goto done;
        }

        PIE_LOG("Verify collection integrity, running at %s (%s)",
                host->hst_name,
                host->hst_fqdn);

        if (storages = pie_cfg_get_hoststg(host->hst_id), storages == NULL)
        {
                PIE_ERR("Failed to load storages");
                goto done;
        }

        PIE_LOG("Storages:");
        for (int i = 0; i < storages->len; i++)
        {
                struct pie_stg_mnt* stg = storages->arr[i];

                if (stg)
                {
                        PIE_LOG("%8s (%d) %-40s type %d",
                                stg->stg.stg_name,
                                stg->stg.stg_id,
                                stg->mnt_path,
                                stg->stg.stg_type);

                        if (stg->stg.stg_type == PIE_STG_THUMB)
                        {
                                stg_thumb = stg;
                        }
                        if (stg->stg.stg_type == PIE_STG_PROXY)
                        {
                                stg_proxy = stg;
                        }
                }
        }

        if (stg_proxy == NULL)
        {
                PIE_WARN("No proxy storage found. Omit consitency check");
        }
        if (stg_thumb == NULL)
        {
                PIE_WARN("No thumb storage found. Omit consitency check");
        }

        /* Scan database */
        ret = sqlite3_prepare_v2(pie_cfg_get_db(), q, -1, &pstmt, NULL);
        if (ret != SQLITE_OK)
        {
                PIE_ERR("Failed to prepare SQL");
                goto done;
        }
        for (;;)
        {
                char path_raw[PIE_PATH_LEN];
                char path_aux[PIE_PATH_LEN];
                struct row r;
                struct pie_stg_mnt* stg_mnt;
                const unsigned char* c;
                int br;
                int got_raw = 1;

                ret = sqlite3_step(pstmt);
                if (ret == SQLITE_DONE)
                {
                        break;
                }
                if (ret == SQLITE_ROW)
                {
                        r.mob_id = sqlite3_column_int64(pstmt, 0);
                        r.stg_id = sqlite3_column_int(pstmt, 1);
                        c = sqlite3_column_text(pstmt, 2);
                        br = sqlite3_column_bytes(pstmt, 2);
                        memcpy(r.min_path, c, br);
                        r.min_path[br] = '\0';
                        mob_ctr++;

                        if (verbose)
                        {
                                PIE_DEBUG("%ld %d %s",
                                          r.mob_id, r.stg_id, r.min_path);
                        }

                        /* check min status */
                        if (r.stg_id == 0)
                        {
                                PIE_WARN("Invalid storage %d for MOB %ld",
                                         r.stg_id,
                                         r.mob_id);
                                err_cnt++;
                        }
                        if (strlen(r.min_path) == 0)
                        {
                                PIE_WARN("Missing path for MOB %ld",
                                         r.mob_id);
                                err_cnt++;
                                continue;
                        }

                        /* Check existence of file referenced by MIN */
                        stg_mnt = PIE_CFG_GET_STORAGE(storages, r.stg_id);
                        if (stg_mnt == NULL)
                        {
                                PIE_WARN("Storage %d for MOB %ld is not mounted",
                                         r.stg_id,
                                         r.mob_id);
                                err_cnt++;
                                continue;
                        }

                        snprintf(path_raw, PIE_PATH_LEN, "%s%s",
                                 stg_mnt->mnt_path,
                                 r.min_path);

                        if (check_path(path_raw))
                        {
                                got_raw = 0;
                        }

                        /* Check proxy files */
                        snprintf(path_aux, PIE_PATH_LEN, "%s/%ld.jpg",
                                 stg_proxy->mnt_path,
                                 r.mob_id);
                        if (check_path(path_aux))
                        {
                                err_cnt++;
                                if (got_raw && dry_run == 0)
                                {
                                        if (tojpg(path_aux, path_raw, -1) == 0)
                                        {
                                                err_fix++;
                                        }
                                }
                        }

                        /* Check thumb files */
                        snprintf(path_aux, PIE_PATH_LEN, "%s/%ld.jpg",
                                 stg_thumb->mnt_path,
                                 r.mob_id);
                        check_path(path_aux);
                        if (check_path(path_aux))
                        {
                                err_cnt++;
                                if (got_raw && dry_run == 0)
                                {
                                        if (tojpg(path_aux, path_raw, max_thumb) == 0)
                                        {
                                                err_fix++;
                                        }
                                }
                        }
                }
                else
                {
                        PIE_ERR("sqlite error: %d", ret);
                }
        }
        sqlite3_finalize(pstmt);

        PIE_LOG("Processed %d MOBs", mob_ctr);
        if (err_cnt)
        {
                PIE_WARN("%d error(s) found", err_cnt);
                PIE_WARN("%d errors where fixed", err_fix);
        }

        status = 0;

done:
        if (host)
        {
                pie_host_free(host);
        }
        if (storages)
        {
                pie_cfg_free_hoststg(storages);
        }
        pie_cfg_close();

        return status;
}

static int check_path(const char* p)
{
        int status = 0;

        if (access(p, F_OK))
        {
                PIE_WARN("File %s can not be accessed",
                         p);
                perror("access");
                status = 1;
        }

        return status;
}

static int tojpg(const char* dst, const char* src, int max)
{
        struct pie_bitmap_f32rgb bm_org;
        struct pie_bitmap_f32rgb bm_dwn;
        struct pie_bitmap_u8rgb bm_out;
        struct pie_io_opt opts;
        int ok;

        opts.qual = PIE_IO_HIGH_QUAL;

        if (ok = pie_io_load(&bm_org, src, &opts), ok)
        {
                return 1;
        }

        if (max> 0)
        {
                if (pie_bm_dwn_smpl(&bm_dwn,
                                    &bm_org,
                                    max,
                                    max))
                {
                        pie_bm_free_f32(&bm_org);
                        return 1;
                }
                pie_bm_free_f32(&bm_org);
                if (pie_bm_conv_bd(&bm_out, PIE_COLOR_8B,
                                   &bm_dwn, PIE_COLOR_32B))
                {
                        pie_bm_free_f32(&bm_org);
                        return 1;
                }
                pie_bm_free_f32(&bm_dwn);
        }
        else
        {
                if (pie_bm_conv_bd(&bm_out, PIE_COLOR_8B,
                                   &bm_org, PIE_COLOR_32B))
                {
                        pie_bm_free_f32(&bm_org);
                        return 1;
                }
                pie_bm_free_f32(&bm_org);
        }

        if (pie_io_jpg_u8rgb_write(dst, &bm_out, 80))
        {
                return 1;
        }

        pie_bm_free_u8(&bm_out);

        return 0;
}
