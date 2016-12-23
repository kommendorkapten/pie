#include <stdio.h>
#include <stdlib.h>
#include "../pie_types.h"
#include "../pie_bm.h"
#include "../io/pie_io_png.h"
    
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
        struct bitmap_u8rgb out;

        /* Create an image. */

        out.width = 100;
        out.height = 100;
        out.color_type = PIE_COLOR_TYPE_RGB;

        bm_alloc_u8(&out);

        for (int y = 0; y < out.height; y++)
        {
                for (int x = 0; x < out.width; x++)
                {
                        struct pixel_u8rgb pixel;

                        pixel.red = (unsigned char)pix(x, out.width);
                        pixel.green = (unsigned char)pix(y, out.height);
                        pixel.blue = 0;

                        pixel_u8rgb_set(&out, x, y, &pixel);
                }
        }

        /* Write the image to a file 'out.png'. */

        png_u8rgb_write("out.png", &out);
        bm_free_u8(&out);

        return 0;
}
