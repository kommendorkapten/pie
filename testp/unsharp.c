#include <stdio.h>
#include "../bm/pie_bm.h"
#include "../bm/pie_bm_jpg.h"
#include "../vendor/timing.h"
#include "../alg/pie_unsharp.h"

int main(int argc, char** argv)
{
        int ret;
        struct pie_bm_f32rgb img;
        struct pie_bm_u8rgb out;
        struct timing t;
        struct pie_unsharp_param p_scr =
                {
                        .radius = 0.5f,
                        .amount = 0.5f,
                        .threshold = 2.0f,
                };
        struct pie_unsharp_param p_web =
                {
                        .radius = 0.3f,
                        .amount = 3.0f,
                        .threshold = 2.0f,
                };
        struct pie_unsharp_param* p;
        long dur;

        p = & p_web;

        if (argc != 2)
        {
                printf("Usage unsharp filename\n");
                return -1;
        }

        timing_start(&t);
        ret = pie_bm_load(&img, argv[1], NULL);
        dur = timing_dur_msec(&t);
        if (ret)
        {
                printf("Error loading media: %d\n", ret);
                return -1;
        }
        printf("Loaded media in %lumsec\n", dur);

        timing_start(&t);

        printf("Using radius: %f\n", p->radius);

        ret = pie_alg_unsharp(img.c_red,
                              img.c_green,
                              img.c_blue,
                              p,
                              img.width,
                              img.height,
                              img.row_stride);
        if (ret)
        {
                abort();
        }

        dur = timing_dur_msec(&t);
        printf("Executed unsharp mask took %lumsec\n", dur);

#if 0
        pie_bm_conv_bd(&out, PIE_BM_COLOR_8B,
                       &img, PIE_BM_COLOR_32B);

        pie_bm_jpg_u8rgb_write("out.jpg", &out, 100);
        pie_bm_free_u8(&out);
#endif


        pie_bm_free_f32(&img);
        return 0;
}
