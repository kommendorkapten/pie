#include <stdio.h>
#include <stdlib.h>
#include "pie_types.h"
#include "pie_io.h"
#include "pie_bm.h"
    
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
        struct bitmap_8rgb fruit;

        /* Create an image. */

        fruit.width = 100;
        fruit.height = 100;

        fruit.pixels = calloc(sizeof(struct pixel_8rgb), 
                              fruit.width * fruit.height);

        for (int y = 0; y < fruit.height; y++)
        {
                for (int x = 0; x < fruit.width; x++)
                {
                        struct pixel_8rgb* pixel = pixel_8rgb_get(&fruit,
                                                                  x, 
                                                                  y);

                        pixel->red = pix(x, fruit.width);
                        pixel->green = pix(y, fruit.height);
                }
        }

        /* Write the image to a file 'fruit.png'. */

        png_8rgb_write("fruit.png", &fruit);

        return 0;
}
