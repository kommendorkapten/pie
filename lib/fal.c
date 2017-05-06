/*
* Copyright (C) 2017 Fredrik Skogman, skogman - at - gmail.com.
*
* The contents of this file are subject to the terms of the Common
* Development and Distribution License (the "License"). You may not use this
* file except in compliance with the License. You can obtain a copy of the
* License at http://opensource.org/licenses/CDDL-1.0. See the License for the
* specific language governing permissions and limitations under the License.
* When distributing the software, include this License Header Notice in each
* file and include the License file at http://opensource.org/licenses/CDDL-1.0.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#define BUF_LEN (1<<13) /* 8 kb */
#define PATH_LEN 1024

int fal_mkdir_tree(const char* dir, const char* new)
{
        char path[PATH_LEN];
        char buf[PATH_LEN];
        char* p = path;
        char* lasts;
        char* t;
        size_t cap = PATH_LEN;
        int bw;
        mode_t mode = 0775;

        strncpy(buf, new, PATH_LEN);
        buf[PATH_LEN - 1] = '\0';
        bw = snprintf(p, cap, "%s", dir);
        cap -= (size_t)bw;
        p += bw;

        t = strtok_r(buf, "/", &lasts);
        while (t != NULL)
        {
                struct stat stat_buf;

                /* Append new directory to base */
                bw = snprintf(p, cap, "/%s", t);
                if (stat(path, &stat_buf))
                {
                        if (errno == ENOENT)
                        {
                                if (mkdir(path, mode))
                                {
                                        return 1;
                                }
                        }
                        else
                        {
                                /* Unknown error */
                                return 1;
                        }
                }

                cap -= (size_t)bw;
                p += bw;
                t = strtok_r(NULL, "/", &lasts);
        }

        return 1;
}

ssize_t fal_copy_fd(int dst, int src)
{
        char buf[BUF_LEN];
        ssize_t br;
        ssize_t bw;
        ssize_t ret = 0;

        for (;;)
        {
                br = read(src, buf, BUF_LEN);
                if (br < 0)
                {
                        ret = -1;
                        break;
                }
                if (br == 0)
                {
                        break;
                }

                ret += br;
                bw = write(dst, buf, (size_t)br);
                if (bw != br)
                {
                        ret = -1;
                        break;
                }
        }

        return ret;
}
