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

#include "pie_catmull.h"
#include <math.h>

static float pie_cm_tj(float,
                       const struct pie_point_2d*,
                       const struct pie_point_2d*);

void pie_mth_catm_rom(struct pie_point_2d* out,
                      const struct pie_point_2d* p,
                      int num_p)
{
        float t0 = 0.0f;
        float t1 = pie_cm_tj(t0, &p[0], &p[1]);
        float t2 = pie_cm_tj(t1, &p[1], &p[2]);
        float t3 = pie_cm_tj(t2, &p[2], &p[3]);
        float step = (t2 - t1) / (float)(num_p - 1);
        float t = t1;

        for (int i = 0; i < num_p; i++)
        {
                struct pie_point_2d a1, a2, a3, b1, b2, c;

                a1.x = ((t1 - t) * p[0].x)/(t1 - t0) + ((t - t0) * p[1].x)/(t1 - t0);
                a1.y = ((t1 - t) * p[0].y)/(t1 - t0) + ((t - t0) * p[1].y)/(t1 - t0);
                a2.x = ((t2 - t) * p[1].x)/(t2 - t1) + ((t - t1) * p[2].x)/(t2 - t1);
                a2.y = ((t2 - t) * p[1].y)/(t2 - t1) + ((t - t1) * p[2].y)/(t2 - t1);
                a3.x = ((t3 - t) * p[2].x)/(t3 - t2) + ((t - t2) * p[3].x)/(t3 - t2);
                a3.y = ((t3 - t) * p[2].y)/(t3 - t2) + ((t - t2) * p[3].y)/(t3 - t2);

                b1.x = ((t2 - t) * a1.x)/(t2 - t0) + ((t - t0) * a2.x)/(t2 - t0);
                b1.y = ((t2 - t) * a1.y)/(t2 - t0) + ((t - t0) * a2.y)/(t2 - t0);
                b2.x = ((t3 - t) * a2.x)/(t3 - t1) + ((t - t1) * a3.x)/(t3 - t1);
                b2.y = ((t3 - t) * a2.y)/(t3 - t1) + ((t - t1) * a3.y)/(t3 - t1);

                c.x = ((t2 - t) * b1.x)/(t2 - t1) + ((t - t1) * b2.x)/(t2 - t1);
                c.y = ((t2 - t) * b1.y)/(t2 - t1) + ((t - t1) * b2.y)/(t2 - t1);

                t += step;
                out[i] = c;
        }
}

void pie_mth_catm_rom_chain(struct pie_point_2d* out,
                            const struct pie_point_2d* p,
                            int num_p,
                            int segment_p)
{
        for (int i = 0; i < num_p - 3; i++)
        {
                pie_mth_catm_rom(out, p + i, segment_p);
                out += segment_p;
        }
}

static float pie_cm_tj(float ti,
                       const struct pie_point_2d* pi,
                       const struct pie_point_2d* pj)
{
        float alpha = 0.5f;
        float tx = pj->x - pi->x;
        float ty = pj->y - pi->y;
        float res;

        tx = tx * tx;
        ty = ty * ty;

        res = powf(tx + ty, 0.5f);
        res = powf(res, alpha) + ti;

        return res;
}
