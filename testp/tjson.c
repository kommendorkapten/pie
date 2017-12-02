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
        unsigned long guard_1 = 0xaaaaaaaaaaaaaaaa;
        static struct pie_dev_settings c;
        unsigned long guard_2 = 0xddddddddbbbbbbbb;
        size_t len;
        char* json = "{\"colort\":0,\"tint\":0,\"expos\":0,\"contr\":0,\"highl\":0,\"shado\":0,\"white\":0,\"black\":0,\"clarity\":{\"amount\": 0,\"rad\": 123099,\"thresh\": 20000},\"vibra\":0,\"satur\":0,\"rot\":0,\"sharp\":{\"amount\": 200000,\"rad\": 100000,\"thresh\": 20000},\"curve_l\":[{\"x\":-262000,\"y\":-188000},{\"x\":0,\"y\":0},{\"x\":263000,\"y\":188000},{\"x\":573000,\"y\":618000},{\"x\":1000000,\"y\":1000000},{\"x\":1428000,\"y\":1382000}],\"curve_r\":[{\"x\":-300000,\"y\":-300000},{\"x\":0,\"y\":0},{\"x\":1000000,\"y\":1000000},{\"x\":1300000,\"y\":1300000}],\"curve_g\":[{\"x\":-300000,\"y\":-300000},{\"x\":0,\"y\":0},{\"x\":1000000,\"y\":1000000},{\"x\":1300000,\"y\":1300000}],\"curve_b\":[{\"x\":-300000,\"y\":-300000},{\"x\":0,\"y\":0},{\"x\":1000000,\"y\":1000000},{\"x\":1300000,\"y\":1300000}]}";

        printf("length: %ld\n", strlen(json) + 1);

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
        printf("%ld\n", len);
        printf("%s\n", buf);

        if (pie_dec_json_settings(&c, json))
        {
                printf("Failed to parse\n");
                printf("%s\n", json);
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

        printf("g1: %lx\n", guard_1);
        printf("g2: %lx\n", guard_2);

        return 0;
}
