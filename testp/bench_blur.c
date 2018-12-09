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

#define KERNEL_LEN 281

long time_gauss(struct pie_bitmap_f32rgb*, float, float*, float*, int);
long time_box(struct pie_bitmap_f32rgb*, float, float*);

int main(int argc, char** argv)
{
        int ret;
        struct pie_bitmap_f32rgb imgl;
        struct pie_bitmap_f32rgb imgs;
        float kernel[KERNEL_LEN];
        float* buf;
        int kernel_len;
        float sigmas[] = {0.5f, 1.0f, 3.0f, 5.0f, 15.0f, 30.0f};
        long timsg[] = {0, 0, 0, 0, 0, 0};
        long timlg[] = {0, 0, 0, 0, 0, 0};
        long timsb[] = {0, 0, 0, 0, 0, 0};
        long timlb[] = {0, 0, 0, 0, 0, 0};
        int count = sizeof(sigmas)/sizeof(float);

        ret = pie_io_load(&imgl, "test-images/large.png", NULL);
        if (ret)
        {
                abort();
        }
        ret = pie_io_load(&imgs, "test-images/sidensvans_small.jpg", NULL);
        if (ret)
        {
                abort();
        }
        /* Make sure memory is paged in */
        buf = malloc(imgl.row_stride * imgl.height * sizeof(float));
        for (int i = 0; i < imgl.row_stride * imgl.height; i++)
        {
                buf[i] = (float)i;
        }

        for (int i = 0; i < count; i++)
        {
                float s = sigmas[i];
                kernel_len = (int)((6.0f * s) + 0.5f);
                if (kernel_len > KERNEL_LEN)
                {
                        kernel_len = KERNEL_LEN;
                }
                if ((kernel_len & 0x1) == 0)
                {
                        kernel_len++;
                }

                printf("Kernel size: %d, sigma: %f\n", kernel_len, s);
                /*
                timlg[i] = time_gauss(&imgl, s, buf, kernel, kernel_len);
                timsg[i] = time_gauss(&imgs, s, buf, kernel, kernel_len);
                */
                timlb[i] = time_box(&imgl, s, buf);
                timsb[i] = time_box(&imgs, s, buf);
        }

        printf("#Large\n");
        printf("#sigma gauss box\n");
        for (int i = 0; i < count; i++)
        {
                printf("%4f %10d %10d\n", sigmas[i], timlg[i], timlb[i]);
        }
        printf("#Small\n");
        printf("#sigma gauss box\n");
        for (int i = 0; i < count; i++)
        {
                printf("%f %10d %10d\n", sigmas[i], timsg[i], timsb[i]);
        }

        free(buf);
        pie_bm_free_f32(&imgl);
        pie_bm_free_f32(&imgs);
        return 0;
}


long time_gauss(struct pie_bitmap_f32rgb* img,
                float s,
                float* buf,
                float* kernel,
                int kernel_len)
{
        struct timing t;
        time_t dur;

        timing_start(&t);
        pie_mth_kernel_sep_gauss(kernel, kernel_len, s * s);
        pie_mth_kernel_sep_apply(img->c_red,
                                 kernel,
                                 kernel_len,
                                 buf,
                                 img->width,
                                 img->height,
                                 img->row_stride);
        pie_mth_kernel_sep_apply(img->c_green,
                                 kernel,
                                 kernel_len,
                                 buf,
                                 img->width,
                                 img->height,
                                 img->row_stride);
        pie_mth_kernel_sep_apply(img->c_blue,
                                 kernel,
                                 kernel_len,
                                 buf,
                                 img->width,
                                 img->height,
                                 img->row_stride);

        dur = timing_dur_usec(&t);
        printf("Executed kernel took %8luusec\n", dur);

        return dur;
}

long time_box(struct pie_bitmap_f32rgb* img, float s, float* buf)
{
        struct timing t;
        time_t dur;

        timing_start(&t);
        pie_mth_box_blur6(img->c_red,
                          buf,
                          s,
                          img->width,
                          img->height,
                          img->row_stride);
        pie_mth_box_blur6(img->c_green,
                          buf,
                          s,
                          img->width,
                          img->height,
                          img->row_stride);
        pie_mth_box_blur6(img->c_blue,
                          buf,
                          s,
                          img->width,
                          img->height,
                          img->row_stride);
        dur = timing_dur_usec(&t);
        printf("Box blur 6 took      %8luusec\n", dur);

        return dur;
}
