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
#include <stddef.h>
#include <sys/time.h>
#include <assert.h>
#include "pie_types.h"
#include "pie_id.h"

pie_id pie_id_from_ts(unsigned char host,
                      unsigned char worker,
                      struct timeval* tv,
                      unsigned char type);

pie_id pie_id_create(unsigned char host,
                     unsigned char worker,
                     enum pie_obj_type type)
{
        struct timeval tv;

        assert(host);
        assert(worker);
        assert(type);

        if (host == 0)
        {
                return 0;
        }
        if (worker == 0)
        {
                return 0;
        }
        if ((int)type == 0)
        {
                return 0;
        }

        if (gettimeofday(&tv, NULL))
        {
                return 0;
        }

        return pie_id_from_ts(host, worker, &tv, (unsigned char)type);
}

/* Internal of a pie_id:
   bit 63    unused (shall be 0)
       24-62 timestamp, in hundreds of a second
       16-23 host
       8-15  worker
       0-7   type.
   Timestamp (39 bits) gives us a timespan of 174 years in 1/100th of a sec.
*/
pie_id pie_id_from_ts(unsigned char host,
                      unsigned char worker,
                      struct timeval* tv,
                      unsigned char type)
{
        pie_id id;
        long ts; /* hundreds of a second, 39bits */
        pie_id ts_mask = 0x7fffffffff;

        ts = (tv->tv_sec - PIE_EPOCH) * 100;
        ts += tv->tv_usec / 10000;

        id = (ts & ts_mask) << 24;
        id |= (pie_id)host << 16;
        id |= (pie_id)worker << 8;
        id |= (pie_id)type;

        return id;
}

pie_id pie_id_from_str(char* s)
{
        char* p;
        pie_id id;

        id = strtol(s, &p, 10);

        if (p == s)
        {
                id = 0;
        }

        return id;
}
