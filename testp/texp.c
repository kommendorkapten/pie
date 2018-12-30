#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../pie_types.h"
#include "../alg/pie_expos.h"
#include "../alg/pie_highl.h"
#include "../bm/pie_bm.h"
#include "../io/pie_io.h"

#define SIZE 512
#define MAX (SIZE-1)

int main(int argc, char** argv)
{
        struct pie_bitmap_f32rgb bmf;
        struct pie_bitmap_u8rgb bmu;

        bmf.width = SIZE;
        bmf.height = 1;
        bmf.color_type = PIE_COLOR_TYPE_RGB;
        bmf.bit_depth = PIE_COLOR_32B;

        bmu.width = SIZE;
        bmu.height = SIZE;
        bmu.color_type = PIE_COLOR_TYPE_RGB;
        bmu.bit_depth = PIE_COLOR_8B;

        pie_bm_alloc_f32(&bmf);
        pie_bm_alloc_u8(&bmu);

        for (int y = 0; y < SIZE; y++)
        {
                for (int x = 0; x < SIZE; x++)
                {
                        bmu.c_red[y * bmu.row_stride + x] = 100;
                        bmu.c_green[y * bmu.row_stride + x] = 100;
                        bmu.c_blue[y * bmu.row_stride + x] = 100;
                }
        }

        for (int x = 0; x < SIZE; x++)
        {
                bmf.c_red[x] = (float)x / (float)MAX;
                bmf.c_green[x] = 0.0f;
                bmf.c_blue[x] = 0.0f;
        }

        /* Plot org as red */
        for (int x = 0; x < SIZE; x++)
        {
                float invy = 1.0f - bmf.c_red[x];
                int y = (int)(invy * MAX);

                bmu.c_red[y * bmu.row_stride + x] = 255;
                bmu.c_green[y * bmu.row_stride + x] = 0;
                bmu.c_blue[y * bmu.row_stride + x] = 0;
        }

        for (int loop = 0; loop < 500; loop++)
        {
                /* apply +0.4 and plot as green */
                pie_alg_expos(bmf.c_red,
                              bmf.c_green,
                              bmf.c_blue,
                              0.4f,
                              SIZE,
                              1,
                              bmf.row_stride);
                for (int x = 0; x < SIZE; x++)
                {
                        float invy = 1.0f - bmf.c_red[x];
                        int y = (int)(invy * MAX);

                        bmu.c_red[y * bmu.row_stride + x] = 0;
                        bmu.c_green[y * bmu.row_stride + x] = 255;
                        bmu.c_blue[y * bmu.row_stride + x] = 0;
                }
        }

        /* apply -1.2  and as plot blue*/
        for (int x = 0; x < SIZE; x++)
        {
                bmf.c_red[x] = (float)x / (float)MAX;
        }
        pie_alg_expos(bmf.c_red,
                      bmf.c_green,
                      bmf.c_blue,
                      -1.2f,
                      SIZE,
                      1,
                      bmf.row_stride);
        for (int x = 0; x < SIZE; x++)
        {
                float invy = 1.0f - bmf.c_red[x];
                int y = (int)(invy * MAX);

                bmu.c_red[y * bmu.row_stride + x] = 0;
                bmu.c_green[y * bmu.row_stride + x] = 0;
                bmu.c_blue[y * bmu.row_stride + x] = 255;
        }

        pie_io_png_u8rgb_write("out.png", &bmu);

        pie_bm_free_f32(&bmf);
        pie_bm_free_u8(&bmu);

        return 0;
}
