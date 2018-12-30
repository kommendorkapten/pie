#include <stdio.h>
#include <stdlib.h>
#include "../pie_types.h"
#include "../bm/pie_bm.h"
#include "../io/pie_io.h"
#include "../lib/timing.h"
#include "../alg/pie_medf3.h"
#include "../alg/pie_unsharp.h"
#include "../alg/pie_cspace.h"
#include "../alg/pie_curve.h"
#include "../math/pie_kernel.h"

int main(int argc, char** argv)
{
        char* in;
        char* out;
        struct pie_bitmap_f32rgb bmf;
        struct pie_bitmap_u8rgb bmu;
        struct timing t;
        struct pie_io_opts opts;
        float* buf;
        int ok;
        char med3 = 0;
        char blur = 1;
        char sharp = 0;
        char sharp_k = 1;
        char curve = 1;

        if (argc != 3)
        {
                printf("Usage: tojpg infile outfile\n");
                return 1;
        }

        in = argv[1];
        out = argv[2];

        opts.qual = PIE_IO_HIGH_QUAL;
        opts.cspace = PIE_IO_LINEAR;

        timing_start(&t);
        if (ok = pie_io_load(&bmf, in, &opts), ok)
        {
                printf("pie_io_load: %d\n", ok);
                return -1;
        }
        printf("Read %s in %luusec\n", in, timing_dur_usec(&t));
        buf = malloc(bmf.height * bmf.row_stride * sizeof(float));

        if (med3)
        {
                timing_start(&t);
                pie_alg_medf3(bmf.c_red,
                              1.0f,
                              buf,
                              bmf.width,
                              bmf.height,
                              bmf.row_stride);
                pie_alg_medf3(bmf.c_green,
                              1.0f,
                              buf,
                              bmf.width,
                              bmf.height,
                              bmf.row_stride);
                pie_alg_medf3(bmf.c_blue,
                              1.0f,
                              buf,
                              bmf.width,
                              bmf.height,
                              bmf.row_stride);
                printf("Med3 in %luusec\n", timing_dur_usec(&t));
        }

        if (blur)
        {
                timing_start(&t);

                float kernel[7]; /* must be odd */
                float sigma = 0.7f;

                /* create separable gauss kernel */
                pie_mth_kernel_sep_gauss(kernel,
                                         7,
                                         sigma * sigma);

                pie_mth_kernel_sep_apply(bmf.c_red,
                                         kernel,
                                         7,
                                         buf,
                                         bmf.width,
                                         bmf.height,
                                         bmf.row_stride);
                pie_mth_kernel_sep_apply(bmf.c_green,
                                         kernel,
                                         7,
                                         buf,
                                         bmf.width,
                                         bmf.height,
                                         bmf.row_stride);
                pie_mth_kernel_sep_apply(bmf.c_blue,
                                         kernel,
                                         7,
                                         buf,
                                         bmf.width,
                                         bmf.height,
                                         bmf.row_stride);

                printf("gauss blur in %luusec\n", timing_dur_usec(&t));
        }

        if (sharp_k)
        {
                timing_start(&t);
                struct pie_kernel3x3 k = {
                        .v = {
                                0.0f, -0.8f, 0.0f,
                                -0.8f, 4.2f, -0.8f,
                                0.0f, -0.8f, 0.0f
                        }
                };

                pie_mth_kernel3x3_apply(bmf.c_red,
                                        &k,
                                        buf,
                                        bmf.width,
                                        bmf.height,
                                        bmf.row_stride);
                pie_mth_kernel3x3_apply(bmf.c_green,
                                        &k,
                                        buf,
                                        bmf.width,
                                        bmf.height,
                                        bmf.row_stride);
                pie_mth_kernel3x3_apply(bmf.c_blue,
                                        &k,
                                        buf,
                                        bmf.width,
                                        bmf.height,
                                        bmf.row_stride);

                printf("sharp (kernel) in %luusec\n", timing_dur_usec(&t));
        }

        if (sharp)
        {
                timing_start(&t);

                struct pie_unsharp_param up = {
                        .amount = 0.85f,
                        .radius = 1.1f,
                        .threshold = 2.0f
                };
                pie_alg_unsharp(bmf.c_red,
                                bmf.c_green,
                                bmf.c_blue,
                                &up,
                                bmf.width,
                                bmf.height,
                                bmf.row_stride);
                printf("sharp in %luusec\n", timing_dur_usec(&t));
        }

        if (curve)
        {
                timing_start(&t);
                struct pie_curve curve_l;
                curve_l.num_p = 6;
                curve_l.cntl_p[0].x = -0.247f;
                curve_l.cntl_p[0].y = -0.193f;
                curve_l.cntl_p[1].x = 0.0f;
                curve_l.cntl_p[1].y = 0.0f;
                curve_l.cntl_p[2].x = 0.248f;
                curve_l.cntl_p[2].y = 0.203f;
                curve_l.cntl_p[3].x = 0.678f;
                curve_l.cntl_p[3].y = 0.763f;
                curve_l.cntl_p[4].x = 1.0f;
                curve_l.cntl_p[4].y = 1.0f;
                curve_l.cntl_p[5].x = 1.323f;
                curve_l.cntl_p[5].y = 1.237f;

                pie_alg_curve(bmf.c_red,
                              bmf.c_green,
                              bmf.c_blue,
                              PIE_CHANNEL_RGB,
                              curve_l.cntl_p,
                              curve_l.num_p,
                              bmf.width,
                              bmf.height,
                              bmf.row_stride);
                printf("curve in %luusec\n", timing_dur_usec(&t));
        }

        timing_start(&t);
        pie_alg_linear_to_srgbv(bmf.c_red,
                                bmf.height * bmf.row_stride);
        pie_alg_linear_to_srgbv(bmf.c_green,
                                bmf.height * bmf.row_stride);
        pie_alg_linear_to_srgbv(bmf.c_blue,
                                bmf.height * bmf.row_stride);
        printf("Convert to sRGB took %luusec\n", timing_dur_usec(&t));

        timing_start(&t);
        if (pie_bm_conv_bd(&bmu, PIE_COLOR_8B,
                           &bmf, PIE_COLOR_32B))
        {
                printf("1.5");
        }
        printf("Converted img to u8 in %luusec\n", timing_dur_usec(&t));

        timing_start(&t);
        if (pie_io_jpg_u8rgb_write(out, &bmu, 100))
        {
                printf("2\n");
                return -1;
        }
        printf("Wrote %s in %luusec\n", out, timing_dur_usec(&t));

        pie_bm_free_u8(&bmu);
        pie_bm_free_f32(&bmf);
        free(buf);

        return 0;
}
