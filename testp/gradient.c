#include <stdio.h>
#include "../bm/pie_bm.h"
#include "../bm/pie_bm_cspace.h"
#include "../bm/pie_bm_png.h"

#define H 100
#define W 512

int main(void)
{
        struct pie_bm_f32rgb bm;
        struct pie_bm_u8rgb out;

        bm.width = W;
        bm.height = 4 * H;
        bm.color_type = PIE_BM_COLOR_TYPE_RGB;

        pie_bm_alloc_f32(&bm);

        for (int y = 0; y < H; y++)
        {
                int ym = y;

                printf("%d\n", ym);
                for (int i = 0; i < W/2; i++)
                {
                        int p = 2 * i + ym * bm.row_stride;

                        float r = (float)(255 - i) / 255.0f;
                        float g = (float)i / 255.0f;
                        float b = 0.0f;

                        bm.c_red[p] = r;
                        bm.c_red[p+1] = r;

                        bm.c_green[p] = g;
                        bm.c_green[p+1] = g;

                        bm.c_blue[p] = b;
                        bm.c_blue[p+1] = b;
                }

        }

        for (int y = 0; y < H; y++)
        {
                int ym = y + H;

                printf("%d\n", ym);
                for (int i = 0; i < W/2; i++)
                {
                        int p = 2 * i + ym * bm.row_stride;

                        float r = (float)(255 - i) / 255.0f;
                        float g = (float)i / 255.0f;
                        float b = 0.0f;

                        r = pie_bm_linear_to_srgb(r);
                        g = pie_bm_linear_to_srgb(g);
                        b = pie_bm_linear_to_srgb(b);

                        bm.c_red[p] = r;
                        bm.c_red[p+1] = r;

                        bm.c_green[p] = g;
                        bm.c_green[p+1] = g;

                        bm.c_blue[p] = b;
                        bm.c_blue[p+1] = b;
                }

        }

        for (int y = 0; y < H; y++)
        {
                int ym = y + 2 * H;

                printf("%d\n", ym);
                for (int i = 0; i < W/2; i++)
                {
                        int p = 2 * i + ym * bm.row_stride;

                        float r = (float)(255 - i) / 255.0f;
                        float g = 0.0f;
                        float b = (float)i / 255.0f;

                        bm.c_red[p] = r;
                        bm.c_red[p+1] = r;

                        bm.c_green[p] = g;
                        bm.c_green[p+1] = g;

                        bm.c_blue[p] = b;
                        bm.c_blue[p+1] = b;
                }

        }

        for (int y = 0; y < H; y++)
        {
                int ym = y + 3 * H;

                printf("%d\n", ym);
                for (int i = 0; i < W/2; i++)
                {
                        int p = 2 * i + ym * bm.row_stride;

                        float r = (float)(255 - i) / 255.0f;
                        float g = 0.0f;
                        float b = (float)i / 255.0f;

                        r = pie_bm_linear_to_srgb(r);
                        g = pie_bm_linear_to_srgb(g);
                        b = pie_bm_linear_to_srgb(b);

                        bm.c_red[p] = r;
                        bm.c_red[p+1] = r;

                        bm.c_green[p] = g;
                        bm.c_green[p+1] = g;

                        bm.c_blue[p] = b;
                        bm.c_blue[p+1] = b;
                }

        }

        pie_bm_conv_bd(&out, PIE_BM_COLOR_8B,
                       &bm, PIE_BM_COLOR_32B);
        pie_bm_png_u8rgb_write("grad.png", &out);
        pie_bm_free_u8(&out);
        pie_bm_free_f32(&bm);
}
