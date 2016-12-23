#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../pie_types.h"
#include "../pie_bm.h"
#include "../io/pie_io.h"
#include "../lib/timing.h"
#include "../alg/pie_kernel.h"
#include "../math/pie_math.h"

#define SEP_LEN 51

/* ceil(6 * sigma) width is typically needed for good performance */
/* sigma = pixels, i.e 0.5px => sigma of 0.5 */

/* Unsharp mask: typically 0.5 to 2.0 amount of ~50-150%
   Local contrast: 30-100 amount of 5-20 % */

int main(int argc, char** argv)
{
        int ret;
        struct bitmap_f32rgb img;
        struct bitmap_u8rgb out;
        struct timing t;
        unsigned long dur;
        float kn[SEP_LEN];
        float* buf;
        float* cpy;
        float s = 60;
        int size;
        float amount = 0.1;

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
        size = img.row_stride * img.height * sizeof(float);        
        buf = malloc(size);
        cpy = malloc(size);
        timing_start(&t);
        pie_kernel_sep_gauss(kn, SEP_LEN, s * s);

        memcpy(cpy, img.c_red, size);
        pie_kernel_sep_apply(cpy,
                             kn,
                             SEP_LEN,
                             buf,
                             img.width,
                             img.height,
                             img.row_stride);

        for (int y = 0; y < img.height; y++)
        {
                for (int x = 0; x < img.width; x++)
                {
                        int p = y * img.row_stride + x;
                        float mask = img.c_red[p] - cpy[p];
                        
                        img.c_red[p] += mask * amount;

                        if (img.c_red[p] < 0.0f)
                        {
                                img.c_red[p] = 0.0f;
                        }
                        else if (img.c_red[p] > 1.0f)
                        {
                                img.c_red[p] = 1.0f;
                        }
                }
        }
        
        memcpy(cpy, img.c_green, size);        
        pie_kernel_sep_apply(cpy,
                             kn,
                             SEP_LEN,
                             buf,
                             img.width,
                             img.height,
                             img.row_stride);

        for (int y = 0; y < img.height; y++)
        {
                for (int x = 0; x < img.width; x++)
                {
                        int p = y * img.row_stride + x;
                        float mask = img.c_green[p] - cpy[p];

                        img.c_green[p] += mask * amount;
                        
                        if (img.c_green[p] < 0.0f)
                        {
                                img.c_green[p] = 0.0f;
                        }
                        else if (img.c_green[p] > 1.0f)
                        {
                                img.c_green[p] = 1.0f;
                        }                        
                }
        }
        
        memcpy(cpy, img.c_blue, size);        
        pie_kernel_sep_apply(cpy,
                             kn,
                             SEP_LEN,
                             buf,
                             img.width,
                             img.height,
                             img.row_stride);        

        for (int y = 0; y < img.height; y++)
        {
                for (int x = 0; x < img.width; x++)
                {
                        int p = y * img.row_stride + x;
                        float mask = img.c_blue[p] - cpy[p];

                        img.c_blue[p] += mask * amount;
                        
                        if (img.c_blue[p] < 0.0f)
                        {
                                img.c_blue[p] = 0.0f;
                        }
                        else if (img.c_blue[p] > 1.0f)
                        {
                                img.c_blue[p] = 1.0f;
                        }
                }
        }
        
        dur = timing_dur_usec(&t);
        printf("Executed blending took %luusec\n", dur);

        bm_conv_bd(&out, PIE_COLOR_8B,
                   &img, PIE_COLOR_32B);

        png_u8rgb_write("out.png", &out);

        printf("Std dev: %f\n", s);
        for (int i = 0; i < SEP_LEN; i++)
        {
                printf("%f ", kn[i]);
        }
        printf("\n");
        
        free(buf);
        free(cpy);
        bm_free_f32(&img);
        bm_free_u8(&out);
        return 0;
}
