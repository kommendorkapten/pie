#include <stdio.h>
#include <stdlib.h>
#include "../pie_types.h"
#include "../bm/pie_bm.h"
#include "../io/pie_io_jpg.h"
    
/* Given "value" and "max", the maximum value which we expect "value"
   to take, this returns an integer between 0 and 255 proportional to
   "value" divided by "max". */

static int pix(int value, int max)
{
        if (value < 0)
        {
                return 0;            
        }

        return (int) (256.0 *((double) (value)/(double) max));
}

int main ()
{
        struct pie_bitmap_u8rgb out;
        int ret;

        /* Create an image. */

        out.width = 120;
        out.height = 120;
        out.color_type = PIE_COLOR_TYPE_RGB;

        pie_bm_alloc_u8(&out);

        for (int y = 0; y < out.height; y++)
        {
                for (int x = 0; x < out.width; x++)
                {
                        struct pie_pixel_u8rgb pixel;

                        pixel.red = (uint8_t)pix(x, out.width);
                        pixel.green = (uint8_t)pix(y, out.height);
                        pixel.blue = 0;

                        pie_pixel_u8rgb_set(&out, x, y, &pixel);
                }
        }

        /* Write the image to a file 'out.png'. */
        ret = pie_io_jpg_u8rgb_write("out.jpg", &out, 90);
        if (ret)
        {
                printf("Failed: %d\n", ret);
        }
        pie_bm_free_u8(&out);

        return 0;
}
