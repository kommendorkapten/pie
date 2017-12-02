#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../encoding/pie_json.h"
#include "../pie_types.h"
#include "../editd/pie_render.h"

#define MAX_DELTA 0.0001f
#define BUF_LEN 1024

#define DIFF(a,b,c) do{ \
                if (fabs(a.c - b.c) > MAX_DELTA) \
                { \
                printf("Diff for %s: %f vs %f\n", #c, a.c, b.c); \
                }} while (0)

static char buf[BUF_LEN];

int main(void)
{
        struct pie_dev_settings s;
        static struct pie_dev_settings c;
        size_t len;

        memset(&c, 0, sizeof(c));

        s.version = 1;
        s.color_temp = 1.0;
        s.tint = 2.0;
        s.exposure = 3.0;
        s.contrast = 4.0;
        s.highlights = 5.0;
        s.shadows = 6.0;
        s.white = 7.0;
        s.black = 8.0;
        s.clarity.amount = 9.0;
        s.clarity.radius = 10.0;
        s.clarity.threshold = 11.0;
        s.vibrance = 0.01f;
        s.saturation = - 0.01f;
        s.rotate = 359;
        s.sharpening.amount = 0.001f;
        s.sharpening.radius = 0.002f;
        s.sharpening.threshold = 0.003f;
        pie_dev_init_curve(&s.curve_l);
        pie_dev_init_curve(&s.curve_r);
        pie_dev_init_curve(&s.curve_g);
        pie_dev_init_curve(&s.curve_b);

        pie_dev_set_to_can_fmt(&s);

        len = pie_enc_json_settings(buf, BUF_LEN, &s);
        buf[len] = '\0';

        if (pie_dec_json_settings(&c, buf))
        {
                printf("Failed to parse\n");
                return 1;
        }

        DIFF(s, c, color_temp);
        DIFF(s, c, tint);
        DIFF(s, c, exposure);
        DIFF(s, c, contrast);
        DIFF(s, c, highlights);
        DIFF(s, c, shadows);
        DIFF(s, c, white);
        DIFF(s, c, black);
        DIFF(s, c, clarity.amount);
        DIFF(s, c, clarity.radius);
        DIFF(s, c, clarity.threshold);
        DIFF(s, c, vibrance);
        DIFF(s, c, saturation);
        DIFF(s, c, rotate);
        DIFF(s, c, sharpening.amount);
        DIFF(s, c, sharpening.radius);
        DIFF(s, c, sharpening.threshold);

        /* RGB */
        if (s.curve_l.num_p != c.curve_l.num_p)
        {
                printf("Diff in numb points l: %d vs %d\n",
                       s.curve_l.num_p,
                       c.curve_l.num_p);
        }

        for (int i = 0; i < s.curve_l.num_p; i++)
        {
                if (fabs(s.curve_l.cntl_p[i].x - c.curve_l.cntl_p[i].x) >  MAX_DELTA)
                {
                        printf("Diff in x@%d: %f vs %f\n",
                               i,
                               s.curve_l.cntl_p[i].x,
                               c.curve_l.cntl_p[i].x);
                }
        }
        for (int i = 0; i < s.curve_l.num_p; i++)
        {
                if (fabs(s.curve_l.cntl_p[i].y - c.curve_l.cntl_p[i].y) >  MAX_DELTA)
                {
                        printf("Diff in y@%d: %f vs %f\n",
                               i,
                               s.curve_l.cntl_p[i].y,
                               c.curve_l.cntl_p[i].y);
                }
        }

        /* R */
        if (s.curve_r.num_p != c.curve_r.num_p)
        {
                printf("Diff in numb points r: %d vs %d\n",
                       s.curve_r.num_p,
                       c.curve_r.num_p);
        }

        for (int i = 0; i < s.curve_r.num_p; i++)
        {
                if (fabs(s.curve_r.cntl_p[i].x - c.curve_r.cntl_p[i].x) >  MAX_DELTA)
                {
                        printf("Diff in x@%d: %f vs %f\n",
                               i,
                               s.curve_r.cntl_p[i].x,
                               c.curve_r.cntl_p[i].x);
                }
        }
        for (int i = 0; i < s.curve_r.num_p; i++)
        {
                if (fabs(s.curve_r.cntl_p[i].y - c.curve_r.cntl_p[i].y) >  MAX_DELTA)
                {
                        printf("Diff in y@%d: %f vs %f\n",
                               i,
                               s.curve_r.cntl_p[i].y,
                               c.curve_r.cntl_p[i].y);
                }
        }

        /* G */
        if (s.curve_g.num_p != c.curve_g.num_p)
        {
                printf("Diff in numb points g: %d vs %d\n",
                       s.curve_g.num_p,
                       c.curve_g.num_p);
        }

        for (int i = 0; i < s.curve_g.num_p; i++)
        {
                if (fabs(s.curve_g.cntl_p[i].x - c.curve_g.cntl_p[i].x) >  MAX_DELTA)
                {
                        printf("Diff in x@%d: %f vs %f\n",
                               i,
                               s.curve_g.cntl_p[i].x,
                               c.curve_g.cntl_p[i].x);
                }
        }
        for (int i = 0; i < s.curve_g.num_p; i++)
        {
                if (fabs(s.curve_g.cntl_p[i].y - c.curve_g.cntl_p[i].y) >  MAX_DELTA)
                {
                        printf("Diff in y@%d: %f vs %f\n",
                               i,
                               s.curve_g.cntl_p[i].y,
                               c.curve_g.cntl_p[i].y);
                }
        }

        /* B */
        if (s.curve_b.num_p != c.curve_b.num_p)
        {
                printf("Diff in numb points b: %d vs %d\n",
                       s.curve_b.num_p,
                       c.curve_b.num_p);
        }

        for (int i = 0; i < s.curve_b.num_p; i++)
        {
                if (fabs(s.curve_b.cntl_p[i].x - c.curve_b.cntl_p[i].x) >  MAX_DELTA)
                {
                        printf("Diff in x@%d: %f vs %f\n",
                               i,
                               s.curve_b.cntl_p[i].x,
                               c.curve_b.cntl_p[i].x);
                }
        }
        for (int i = 0; i < s.curve_b.num_p; i++)
        {
                if (fabs(s.curve_b.cntl_p[i].y - c.curve_b.cntl_p[i].y) >  MAX_DELTA)
                {
                        printf("Diff in y@%d: %f vs %f\n",
                               i,
                               s.curve_b.cntl_p[i].y,
                               c.curve_b.cntl_p[i].y);
                }
        }

        return 0;
}
