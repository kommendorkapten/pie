#include <stdio.h>
#include "../pie_types.h"
#include "../bm/pie_bm.h"
#include "../io/pie_io.h"
#include "../lib/timing.h"
#include "../alg/pie_contr.h"

int main(int argc, char** argv)
{
        int ret;
        struct pie_bitmap_f32rgb img;
        struct pie_bitmap_u8rgb out;
        struct timing t;
        long dur;
        float amount = 1.5f;

        if (argc != 2)
        {
                printf("Usage contr filename\n");
                return -1;
        }

        timing_start(&t);
        ret = pie_io_load(&img, argv[1], NULL);
        dur = timing_dur_usec(&t);
        if (ret)
        {
                printf("Error loading media: %d\n", ret);
                return -1;
        }
        printf("Loaded media in %luusec\n", dur);

        timing_start(&t);
        pie_alg_contr(img.c_red,
                      amount,
                      img.width,
                      img.height,
                      img.row_stride);
        pie_alg_contr(img.c_green,
                      amount,
                      img.width,
                      img.height,
                      img.row_stride);
        pie_alg_contr(img.c_blue,
                      amount,
                      img.width,
                      img.height,
                      img.row_stride);

        dur = timing_dur_usec(&t);
        printf("Calculate contrast took %luusec\n", dur);

        pie_bm_conv_bd(&out, PIE_COLOR_8B,
                       &img, PIE_COLOR_32B);

        timing_start(&t);
        pie_io_png_u8rgb_write("out.png", &out);
        dur = timing_dur_usec(&t);
        printf("Wrote output in %luusec\n", dur);

        pie_bm_free_f32(&img);
        pie_bm_free_u8(&out);

        return 0;
}
