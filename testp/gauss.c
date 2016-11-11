#include <stdio.h>
#include <stdlib.h>
#include "../pie_types.h"
#include "../pie_bm.h"
#include "../io/pie_io.h"
#include "../lib/timing.h"
#include "../alg/pie_kernel.h"

/* ceil(6 * sigma) width is typically needed for good performance */
/* sigma = pixels, i.e 0.5px => sigma of 0.5 */

int main(int argc, char** argv)
{
        int ret;
        struct bitmap_f32rgb img;
        struct bitmap_u8rgb out;
        struct timing t;
        unsigned long dur;
        struct pie_kernel3x3 k3;
        struct pie_kernel5x5 k5;
        float s1 = 0.84089642;
        float s2 = 0.54089642;
        float* buf;
        float sum = 0.0f;
        float s = 100.0;
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
        buf = malloc(img.row_stride * img.height * sizeof(float));
        timing_start(&t);
        pie_kernel3x3_gauss(&k3,
                            s * s);
        pie_kernel5x5_gauss(&k5,
                            s * s);
#if 0
        pie_kernel3x3_apply(img.c_red,
                            &k3,
                            buf,
                            img.width,
                            img.height,
                            img.row_stride);
        pie_kernel3x3_apply(img.c_green,
                            &k3,
                            buf,
                            img.width,
                            img.height,
                            img.row_stride);
        pie_kernel3x3_apply(img.c_blue,
                            &k3,
                            buf,
                            img.width,
                            img.height,
                            img.row_stride);
#else
        pie_kernel5x5_apply(img.c_red,
                            &k5,
                            buf,
                            img.width,
                            img.height,
                            img.row_stride);
        pie_kernel5x5_apply(img.c_green,
                            &k5,
                            buf,
                            img.width,
                            img.height,
                            img.row_stride);
        pie_kernel5x5_apply(img.c_blue,
                            &k5,
                            buf,
                            img.width,
                            img.height,
                            img.row_stride);
#endif

        dur = timing_dur_usec(&t);
        printf("Executed kernel took %luusec\n", dur);

        bm_conv_bd(&out, PIE_COLOR_8B,
                   &img, PIE_COLOR_32B);

        png_u8rgb_write("out.png", &out);

        printf("Std dev: %f\n", s);
        for (int i = 0; i < 9; i++)
        {
                sum += k3.v[i];
        }
        for (int y = 0; y < 3; y++)
        {
                for (int x = 0; x < 3; x++)
                {
                        printf("%f ", k3.v[y*3 + x]);
                }
                printf("\n");
        }
        printf("Sum (3x3): %f\n", sum);
        sum = 0;
        for (int i = 0; i < 25; i++)
        {
                sum += k5.v[i];
        }
        for (int y = 0; y < 5; y++)
        {
                for (int x = 0; x < 5; x++)
                {
                        printf("%f ", k5.v[y*5 + x]);
                }
                printf("\n");
        }
        printf("Sum (5x5): %f\n", sum);

        free(buf);
        bm_free_f32(&img);
        bm_free_u8(&out);
        return 0;
}
