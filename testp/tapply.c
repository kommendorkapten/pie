#include <stdio.h>
#include <stdlib.h>
#include "../bm/pie_bm.h"
#include "../bm/pie_bm_jpg.h"
#include "../bm/pie_bm_cspace.h"
#include "../vendor/timing.h"
#include "../encoding/pie_rgba.h"
#include "../alg/pie_hist.h"
#include "../alg/pie_render.h"

int main(int argc, char** argv)
{
        struct pie_bm_f32rgb img;
        struct pie_bm_u8rgb out;
        struct timing t;
        struct pie_dev_settings settings;
        struct pie_histogram hist;
        struct pie_bm_opts opts = {
                .qual = PIE_BM_HIGH_QUAL,
#if _PIE_EDIT_LINEAR
                .cspace = PIE_BM_LINEAR
#else
                .cspace = PIE_BM_SRGB
#endif
        };
        char* out_name = "out.jpg";
        float* buf;
        long dur;
        int ret;

        if (argc != 2)
        {
                printf("Usage tapply filename\n");
                return -1;
        }

        timing_start(&t);
        ret = pie_bm_load(&img, argv[1], &opts);
        dur = timing_dur_msec(&t);
        if (ret)
        {
                printf("Error loading media: %d\n", ret);
                return -1;
        }
        printf("Loaded media in %ldmsec\n", dur);

        buf = malloc(img.width * img.row_stride * sizeof(float) + 8);
        pie_alg_init_settings(&settings, img.width, img.height);

        settings.saturation = 1.0f;
        settings.contrast = 1.0f;
        timing_start(&t);
        pie_alg_render(&img, buf, &settings);
        dur = timing_dur_msec(&t);
        printf("Rendered in %ldmsec\n", dur);

#if _PIE_EDIT_LINEAR
        timing_start(&t);
        pie_bm_linear_to_srgbv(img.c_red,
                               img.height * img.row_stride);
        pie_bm_linear_to_srgbv(img.c_green,
                               img.height * img.row_stride);
        pie_bm_linear_to_srgbv(img.c_blue,
                               img.height * img.row_stride);
        printf("Convert to sRGB took %lums\n", timing_dur_msec(&t));
#endif /* _PIE_EDIT_LINEAR */

        timing_start(&t);
        pie_bm_conv_bd(&out, PIE_BM_COLOR_8B,
                       &img, PIE_BM_COLOR_32B);
        dur = timing_dur_msec(&t);
        printf("Converted media to 8bit in %lumsec\n", dur);

        timing_start(&t);
        pie_bm_jpg_u8rgb_write(out_name, &out, 95);
        dur = timing_dur_msec(&t);
        printf("Wrote %s in %ldmsec\n", out_name, dur);

        timing_start(&t);
        pie_enc_bm_rgba((unsigned char*)buf, &img, PIE_IMAGE_TYPE_PRIMARY);
        dur = timing_dur_msec(&t);
        printf("RGBA encode in %ldmsec\n", dur);

        timing_start(&t);
        pie_alg_hist_lum(&hist, &img);
        dur = timing_dur_msec(&t);
        printf("Created LUM hist in %ldmsec\n", dur);

        timing_start(&t);
        pie_alg_hist_rgb(&hist, &img);
        dur = timing_dur_msec(&t);
        printf("Created RGB hist in %ldmsec\n", dur);

        free(buf);
        pie_bm_free_f32(&img);
        pie_bm_free_u8(&out);

        return 0;
}
