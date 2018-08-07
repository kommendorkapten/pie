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

#include <assert.h>
#include "timing.h"

void timing_start(struct timing* t)
{
        int res = gettimeofday(&t->tv, NULL);

        assert(res == 0);
}

long timing_dur_sec(const struct timing* t)
{
        struct timeval now;
        int res;

        res = gettimeofday(&now, NULL);
        assert(res == 0);

        return (long)(now.tv_sec - t->tv.tv_sec);
}

long timing_dur_usec(const struct timing* t)
{
        struct timeval now;
        int res;

        res = gettimeofday(&now, NULL);
        assert(res == 0);

        return (long)((now.tv_sec * 1000000 + now.tv_usec) -
                      (t->tv.tv_sec * 1000000 + t->tv.tv_usec));
}

long timing_dur_msec(const struct timing* t)
{
        time_t usec = timing_dur_usec(t);

        return (long)(usec / 1000);
}

long timing_current_millis(void)
{
        struct timeval now;
        int res;

        res = gettimeofday(&now, NULL);
        assert(res == 0);

        return (long)(now.tv_sec * 1000 + now.tv_usec / 1000);
}
