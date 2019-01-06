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
#include <openssl/evp.h>
#include <fcntl.h>
#include "../io/pie_io.h"
#include "../bm/pie_bm.h"
#include "../pie_types.h"
#include "../pie_id.h"
#include "../pie_log.h"
#include "../cfg/pie_cfg.h"
#include "../dm/pie_storage.h"
#include "../dm/pie_host.h"
#include "../dm/pie_min.h"
#include "../lib/llist.h"
#include "../alg/pie_cspace.h"

#define BUF_LEN 4096

struct row
{
        pie_id mob_id;
        int stg_id;
        char min_path[PIE_PATH_LEN];
};

struct errors
{
        int err_cnt;
        int err_fix;
};

static int cksum_verify = 0;
static int verbose = 0;
static int dry_run = 0;

static struct errors check_min(pie_id mob_id, int stg_id, const char* path);
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
        int c;
        int max_thumb;
        long lval;

        while ((c = getopt(argc, argv, "vdx")) != -1)
        {
                switch (c)
                {
                case 'd':
                        dry_run = 1;
                        PIE_LOG("Entering dry run. No modificatons will be done");
                        break;
                case 'x':
                        cksum_verify = 1;
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
        max_thumb = (int)lval;

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

                        struct errors err;
                        err = check_min(r.mob_id, r.stg_id, path_raw);
                        err_cnt += err.err_cnt;
                        err_fix += err.err_fix;

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
        struct pie_bitmap_f32rgb bm_in;
        struct pie_bitmap_u8rgb bm_out;
        struct pie_io_opts opts;
        int ok;

        opts.qual = PIE_IO_HIGH_QUAL;
#if _PIE_EDIT_LINEAR
        opts.cspace = PIE_IO_LINEAR;
#else
        opts.cspace = PIE_IO_SRGB;
#endif

        if (ok = pie_io_load(&bm_in, src, &opts), ok)
        {
                return 1;
        }

        if (max> 0)
        {
                struct pie_bitmap_f32rgb new;

                if (pie_bm_dwn_smpl(&new,
                                    &bm_in,
                                    max,
                                    max))
                {
                        pie_bm_free_f32(&bm_in);
                        return 1;
                }

                pie_bm_free_f32(&bm_in);
                bm_in = new;
        }

#if _PIE_EDIT_LINEAR
        pie_alg_linear_to_srgbv(bm_in.c_red,
                                bm_in.height * bm_in.row_stride);
        pie_alg_linear_to_srgbv(bm_in.c_green,
                                bm_in.height * bm_in.row_stride);
        pie_alg_linear_to_srgbv(bm_in.c_blue,
                                bm_in.height * bm_in.row_stride);
#endif /* _PIE_EDIT_LINEAR */

        if (pie_bm_conv_bd(&bm_out, PIE_COLOR_8B,
                           &bm_in, PIE_COLOR_32B))
        {
                pie_bm_free_f32(&bm_in);
                return 1;
        }

        if (pie_io_jpg_u8rgb_write(dst, &bm_out, 80))
        {
                return 1;
        }

        pie_bm_free_f32(&bm_in);
        pie_bm_free_u8(&bm_out);

        return 0;
}

static struct errors check_min(pie_id mob_id, int stg_id, const char* path)
{
        char buf[BUF_LEN];
        char md_hex[2 * EVP_MAX_MD_SIZE + 1];
        unsigned char md_sum[EVP_MAX_MD_SIZE];
        struct errors ret = {.err_cnt = 0, .err_fix = 0};
        EVP_MD_CTX* mdctx = EVP_MD_CTX_create();
        const EVP_MD* md_alg = EVP_sha1();
        ENGINE* impl = NULL;
        struct llist* l;
        struct lnode* n;

        (void)stg_id;

        l = pie_min_find_mob(pie_cfg_get_db(), mob_id);
        if (l == NULL)
        {
                PIE_WARN("DB error");
                goto done;
        }

        n = llist_head(l);
        while (n)
        {
                struct pie_min* min = n->data;
                long br = 0;
                ssize_t nb;
                int fd;
                unsigned int md_len;

                fd = open(path, O_RDONLY);

                /* Read file, calc hash and size */
                EVP_DigestInit_ex(mdctx, md_alg, impl);
                while ((nb = read(fd, buf, BUF_LEN)) > 0)
                {
                        if (cksum_verify)
                        {
                                EVP_DigestUpdate(mdctx, buf, nb);
                        }

                        br += (long)nb;
                }
                EVP_DigestFinal_ex(mdctx, md_sum, &md_len);

                /* Create hex and compare */
                if (cksum_verify)
                {
                        for (unsigned int i = 0; i < md_len; i++)
                        {
                                snprintf(md_hex + i * 2,
                                         MIN_HASH_LEN,
                                         "%02x",
                                         md_sum[i]);
                        }
                        if (strcmp(min->min_sha1_hash, md_hex))
                        {
                                PIE_WARN("Incosistent hash: %ld", min->min_id);
                                ret.err_cnt++;
                        }
                }

                if (br != min->min_size)
                {
                        PIE_WARN("Min %ld, size mismatch %ld vs %ld",
                                 min->min_id,
                                 min->min_size,
                                 br);
                        ret.err_cnt++;
                }
                if (!dry_run)
                {
                        min->min_size = br;

                        if (pie_min_update(pie_cfg_get_db(), min))
                        {
                                PIE_WARN("Could not update MIN %ld",
                                         min->min_id);
                        }
                        else
                        {
                                ret.err_fix++;
                        }
                }

                close(fd);
                pie_min_free(min);
                n = n->next;
        }
        llist_destroy(l);

done:
        EVP_MD_CTX_destroy(mdctx);

        return ret;
}
