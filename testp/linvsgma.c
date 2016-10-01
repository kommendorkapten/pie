#include "../pie_types.h"
#include "../pie_bm.h"
#include "../io/pie_io.h"
#include "../pie_cspace.h"

#define USE_PNG 1

int main(void)
{
#if USE_PNG
        char* out_lin = "lin.png";
        char* out_gma = "gma.png";
#else
        char* out_lin = "lin.jpg";
        char* out_gma = "gma.jpg";
#endif
        struct bitmap_u8rgb out;
        struct bitmap_f32rgb img;

        img.width = 1024;
        img.height = 100;
        img.color_type = PIE_COLOR_TYPE_RGB;

        bm_alloc_f32(&img);

        for (unsigned int y = 0; y < img.height; y++)
        {
                for (unsigned int x = 0; x < img.width; x++)
                {
                        unsigned int p = y * img.row_stride + x;
                        float c = x / (4 * 255.0);

                        img.c_red[p] = c;
                        img.c_green[p] = c;
                        img.c_blue[p] = c;
                }
        }

        bm_conv_bd(&out, PIE_COLOR_8B,
                   &img, PIE_COLOR_32B);
#if USE_PNG
        png_u8rgb_write(out_lin, &out);
#else
        jpg_u8rgb_write(out_lin, &out, 100);
#endif

        /* Convert to sRGB */
        for (unsigned int y = 0; y < img.height; y++)
        {
                linear_to_srgbv(img.c_red + y * img.row_stride, img.width);
                linear_to_srgbv(img.c_green + y * img.row_stride, img.width);
                linear_to_srgbv(img.c_blue + y * img.row_stride, img.width);
        }

        bm_free_u8(&out);
        bm_conv_bd(&out, PIE_COLOR_8B,
                   &img, PIE_COLOR_32B);
#if USE_PNG
        png_u8rgb_write(out_gma, &out);
#else
        jpg_u8rgb_write(out_gma, &out, 100);
#endif

        bm_free_f32(&img);
        bm_free_u8(&out);
        return 0;
}
