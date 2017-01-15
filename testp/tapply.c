#include <stdio.h>
#include <stdlib.h>
#include "../pie_types.h"
#include "../pie_bm.h"
#include "../io/pie_io.h"
#include "../lib/timing.h"
#include "../exe/pie_render.h"

int main(int argc, char** argv)
{
        struct bitmap_f32rgb img;
        struct bitmap_u8rgb out;
        struct timing t;
        struct pie_img_settings settings;
        char* out_name = "out.jpg";
        float* buf;
        unsigned long dur;
        int ret;

        pie_img_init_settings(&settings);
        
        if (argc != 2)
        {
                printf("Usage contr filename\n");
                return -1;
        }

        timing_start(&t);
        ret = pie_io_load(&img, argv[1]);
        dur = timing_dur_usec(&t);
        if (ret)
        {
                printf("Error loading media: %d\n", ret);
                return -1;
        }
        printf("Loaded media in %luusec\n", dur);

        buf = malloc(img.width * img.row_stride * sizeof(float));

        settings.exposure = 1.0f;
        pie_img_render(&img, buf, &settings);
        
        timing_start(&t);
        bm_conv_bd(&out, PIE_COLOR_8B,
                   &img, PIE_COLOR_32B);
        dur = timing_dur_usec(&t);
        printf("Converted media to 8bit in %luusec\n", dur);

        timing_start(&t);
        jpg_u8rgb_write(out_name, &out, 95);
        dur = timing_dur_usec(&t);
        printf("Wrote %s in %luusec\n", out_name, dur);

        free(buf);
        bm_free_f32(&img);
        bm_free_u8(&out);

        return 0;
}
