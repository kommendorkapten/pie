#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../bm/pie_bm_cspace.h"
#include "../vendor/timing.h"

#define NUM_P 10
#define BENCH 0

int main(void)
{
#if BENCH
#define SIZE (4000 * 6000)
        struct timing t;
        float* r0 = malloc(SIZE * sizeof(float));
        float* r1 = malloc(SIZE * sizeof(float));

        for (int i = 0; i < SIZE; i++)
        {
                r0[i] = (float)i / (float)SIZE;
                r1[i] = (float)i / (float)SIZE;
        }
        timing_start(&t);
        for (int i = 0; i < SIZE; i++)
        {
                r0[i] = pie_bm_srgb_to_linear(r0[i]);
        }
        printf("Pow: %ldmsec\n", timing_dur_msec(&t));

        timing_start(&t);
        for (int i = 0; i < SIZE; i++)
        {
                r1[i] = pie_bm_srgb_to_linearp(r1[i]);
        }
        printf("Pol: %ldmsec\n", timing_dur_msec(&t));

        float max = 0.0f;
        for (int i = 0; i < SIZE; i++)
        {
                float d = fabsf(r0[i] - r1[i]);
                if (d > max)
                {
                        max = d;
                }
        }

        printf("Max: %f\n", max);
#else
        float* dpow = malloc(NUM_P * sizeof(float));
        float* dpol = malloc(NUM_P * sizeof(float));

        for (int i = 0; i < NUM_P; i++)
        {
                float f = (float)i / (float)NUM_P;

#if 0
                dpow[i] = pie_bm_srgb_to_linear(f);
                dpol[i] = pie_bm_srgb_to_linearp(f);
#else
                dpow[i] = pie_bm_linear_to_srgb(f);
                dpol[i] = pie_bm_linear_to_srgbp(f);
#endif
        }

        for (int i = 0; i < NUM_P; i++)
        {
#if 1
                float d = dpow[i] - dpol[i];

                printf("%2.6f %f %f %f %f\n",
                       fabs((d * 100.0f) / dpow[i]),
                       (float)i / (float)NUM_P,
                       dpow[i],
                       dpol[i],
                       d);
#else
                int w = (dpow[i] * 65535.0f + 0.5f);
                int l = (dpol[i] * 65535.0f + 0.5f);
                int d = abs(w - l);
                printf("%d\n", d);
#endif
        }

        free(dpow);
        free(dpol);
#endif
        return 0;
}
