#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../pie_types.h"
#include "../bm/pie_bm.h"
#include "../io/pie_io.h"
#include "../lib/timing.h"
#include "../math/pie_kernel.h"
#include "../math/pie_blur.h"
#include "../alg/pie_cspace.h"

#define KERNEL_LEN 281

/* ceil(6 * sigma) width is typically needed for good performance */
/* sigma = pixels, i.e 0.5px => sigma of 0.5 */

/*
Sparc: box always faster.
amd64: use gauss if sigma is <= 5. VERIFY!!!!
*/


int main(int argc, char** argv)
{
        int ret;
        struct pie_bitmap_f32rgb img;
        struct pie_bitmap_u8rgb out;
        struct timing t;
        long dur;
        float kernel[KERNEL_LEN];
        /* float s1 = 0.84089642; */
        /* float s2 = 0.54089642; */
        float* buf;
        float s = 6.0f;
        int kernel_len = (int)((6.0f * s) + 0.5f);
        int size;
        struct pie_io_opts opts = {
                .qual = PIE_IO_HIGH_QUAL,
                .cspace = PIE_IO_LINEAR
        };

        if (kernel_len > KERNEL_LEN)
        {
                kernel_len = KERNEL_LEN;
        }

        if ((kernel_len & 0x1) == 0)
        {
                kernel_len++;
        }

        if (argc != 2)
        {
                printf("Usage gauss filename\n");
                return -1;
        }

        timing_start(&t);
        ret = pie_io_load(&img, argv[1], &opts);
        dur = timing_dur_msec(&t);
        if (ret)
        {
                printf("Error loading media: %d\n", ret);
                return -1;
        }
        printf("Loaded media in %lumsec\n", dur);

        size = (int)(img.row_stride * img.height * sizeof(float));
        buf = malloc(size);
        printf("Apply Gaussian blur with sigma: %f and kernel size: %d\n",
               s, kernel_len);
        timing_start(&t);

        pie_mth_kernel_sep_gauss(kernel, kernel_len, s * s);
        pie_mth_kernel_sep_apply(img.c_red,
                                 kernel,
                                 kernel_len,
                                 buf,
                                 img.width,
                                 img.height,
                                 img.row_stride);
        pie_mth_kernel_sep_apply(img.c_green,
                                 kernel,
                                 kernel_len,
                                 buf,
                                 img.width,
                                 img.height,
                                 img.row_stride);
        pie_mth_kernel_sep_apply(img.c_green,
                                 kernel,
                                 kernel_len,
                                 buf,
                                 img.width,
                                 img.height,
                                 img.row_stride);

        dur = timing_dur_msec(&t);
        printf("Executed kernel took %lumsec\n", dur);

        timing_start(&t);
        pie_alg_linear_to_srgbv(img.c_red,
                                img.height * img.row_stride);
        pie_alg_linear_to_srgbv(img.c_green,
                                img.height * img.row_stride);
        pie_alg_linear_to_srgbv(img.c_blue,
                                img.height * img.row_stride);
        printf("Convert to sRGB took %lumsec\n", timing_dur_msec(&t));

        pie_bm_conv_bd(&out, PIE_COLOR_8B,
                       &img, PIE_COLOR_32B);
        pie_io_jpg_u8rgb_write("gauss.jpg", &out, 100);
        pie_bm_free_u8(&out);
        pie_bm_free_f32(&img);

        ret = pie_io_load(&img, argv[1], &opts);
        /* BOX Blur 6 */
        timing_start(&t);
        pie_mth_box_blur6(img.c_red,
                          buf,
                          s,
                          img.width,
                          img.height,
                          img.row_stride);
        pie_mth_box_blur6(img.c_green,
                          buf,
                          s,
                          img.width,
                          img.height,
                          img.row_stride);
        pie_mth_box_blur6(img.c_blue,
                          buf,
                          s,
                          img.width,
                          img.height,
                          img.row_stride);

        dur = timing_dur_msec(&t);
        printf("Box blur 6 took %lumsec\n", dur);

        timing_start(&t);
        pie_alg_linear_to_srgbv(img.c_red,
                                img.height * img.row_stride);
        pie_alg_linear_to_srgbv(img.c_green,
                                img.height * img.row_stride);
        pie_alg_linear_to_srgbv(img.c_blue,
                                img.height * img.row_stride);
        printf("Convert to sRGB took %lumsec\n", timing_dur_msec(&t));

        pie_bm_conv_bd(&out, PIE_COLOR_8B,
                       &img, PIE_COLOR_32B);
        pie_io_jpg_u8rgb_write("box.jpg", &out, 100);
        pie_bm_free_u8(&out);

        free(buf);
        pie_bm_free_f32(&img);
        return 0;
}
