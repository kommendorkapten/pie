#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../pie_types.h"
#include "../bm/pie_bm.h"
#include "../io/pie_io.h"
#include "../lib/timing.h"
#include "../alg/pie_kernel.h"
#include "../math/pie_blur.h"

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
        struct bitmap_f32rgb img;
        struct bitmap_u8rgb out;
        struct timing t;
        time_t dur;
        float kernel[KERNEL_LEN];
        /* float s1 = 0.84089642; */
        /* float s2 = 0.54089642; */
        float* buf;
        float* tmp;
        float s = 2.0f;
        int kernel_len = (int)((6.0f * s) + 0.5f);
        int size;

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
        tmp = malloc(size);
        printf("Apply Gaussian blur with sigma: %f and kernel size: %d\n",
               s, kernel_len);
        timing_start(&t);

        pie_kernel_sep_gauss(kernel, kernel_len, s * s);
        memcpy(tmp, img.c_red, size);
        pie_kernel_sep_apply(tmp,
                             kernel,
                             kernel_len,
                             buf,
                             img.width,
                             img.height,
                             img.row_stride);
        for (int i = 0; i < size/4; i++)
        {
                img.c_red[i] -= tmp[i];
                if (img.c_red[i] < 0.0f)
                {
                        img.c_red[i] = 0.0f;
                }
                if (img.c_red[i] > 1.0f)
                {
                        img.c_red[i] = 1.0f;
                }
        }


        memcpy(tmp, img.c_green, size);
        pie_kernel_sep_apply(tmp,
                             kernel,
                             kernel_len,
                             buf,
                             img.width,
                             img.height,
                             img.row_stride);
        for (int i = 0; i < size/4; i++)
        {
                img.c_green[i] -= tmp[i];                
                if (img.c_green[i] < 0.0f)
                {
                        img.c_green[i] = 0.0f;
                }
                if (img.c_green[i] > 1.0f)
                {
                        img.c_green[i] = 1.0f;
                }
        }

        memcpy(tmp, img.c_blue, size);
        pie_kernel_sep_apply(tmp,
                             kernel,
                             kernel_len,
                             buf,
                             img.width,
                             img.height,
                             img.row_stride);        
        for (int i = 0; i < size/4; i++)
        {
                img.c_blue[i] -= tmp[i];                
                if (img.c_blue[i] < 0.0f)
                {
                        img.c_blue[i] = 0.0f;
                }
                if (img.c_blue[i] > 1.0f)
                {
                        img.c_blue[i] = 1.0f;
                }
        }

        dur = timing_dur_usec(&t);
        printf("Executed kernel took %8luusec\n", dur);

        bm_conv_bd(&out, PIE_COLOR_8B,
                   &img, PIE_COLOR_32B);
        png_u8rgb_write("gauss.png", &out);
        bm_free_u8(&out);
        bm_free_f32(&img);
        ret = pie_io_load(&img, argv[1]);

        /* BOX Blur 6 */
        timing_start(&t);
        memcpy(tmp, img.c_red, size);
        pie_box_blur6(tmp,
                      buf,
                      s,
                      img.width,
                      img.height,
                      img.row_stride);
        for (int i = 0; i < size/4; i++)
        {
                img.c_red[i] -= tmp[i];
                if (img.c_red[i] < 0.0f)
                {
                        img.c_red[i] = 0.0f;
                }
                if (img.c_red[i] > 1.0f)
                {
                        img.c_red[i] = 1.0f;
                }
        }

        memcpy(tmp, img.c_green, size);        
        pie_box_blur6(tmp,
                      buf,
                      s,
                      img.width,
                      img.height,
                      img.row_stride);
        for (int i = 0; i < size/4; i++)
        {
                img.c_green[i] -= tmp[i];
                if (img.c_green[i] < 0.0f)
                {
                        img.c_green[i] = 0.0f;
                }
                if (img.c_green[i] > 1.0f)
                {
                        img.c_green[i] = 1.0f;
                }
        }

        memcpy(tmp, img.c_blue, size);
        pie_box_blur6(tmp,
                      buf,    
                      s,
                      img.width,
                      img.height,
                      img.row_stride);
        for (int i = 0; i < size/4; i++)
        {
                img.c_blue[i] -= tmp[i];
                if (img.c_blue[i] < 0.0f)
                {
                        img.c_blue[i] = 0.0f;
                }
                if (img.c_blue[i] > 1.0f)
                {
                        img.c_blue[i] = 1.0f;
                }
        }
        
        dur = timing_dur_usec(&t);
        printf("Box blur 6 took      %8luusec\n", dur);
        bm_conv_bd(&out, PIE_COLOR_8B,
                   &img, PIE_COLOR_32B);
        png_u8rgb_write("box.png", &out);
        bm_free_u8(&out);
        
        free(buf);
        bm_free_f32(&img);
        return 0;
}
