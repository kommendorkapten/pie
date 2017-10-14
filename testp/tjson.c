#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../encoding/pie_json.h"
#include "../pie_types.h"

#define MAX_DELTA 0.0001f
#define BUF_LEN 1024

#define DIFF(a,b,c) do{ \
                if (fabs(a.c - b.c) > MAX_DELTA) \
                { \
                printf("Diff for %s: %f vs %f\n", #c, a.c, b.c); \
                }} while (0)

int main(void)
{
        char buf[BUF_LEN];
        struct pie_dev_settings s;
        struct pie_dev_settings c;
        size_t len;

        memset(&c, 0, sizeof(c));
        
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
        s.vibrance = 0.01;
        s.saturation = - 0.01;
        s.rotate = 359;
        s.sharpening.amount = 0.001;
        s.sharpening.radius = 0.002;
        s.sharpening.threshold = 0.003;

        len = pie_enc_json_settings(buf, BUF_LEN, &s);
        buf[len] = '\0';
        printf("%ld\n", len);
        printf("%s\n", buf);

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
}
