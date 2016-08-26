#include <stdio.h>
#include <stdlib.h>
#include "pie.h"

int main(void)
{
        char* f = "out.png";
        struct bitmap_8rgb src;
        struct bitmap_f32rgb dst;
        
        src.height = 10;
        src.width = 10;
        src.color_type = PIE_COLOR_TYPE_RGB;
        bm_alloc_8(&src);

        for (unsigned int y = 0; y < src.height; y++)
        {
                for (unsigned int x = 0; x < src.width; x++)
                {
                        uint8_t c = y * src.width + x;

                        src.c_red[y * src.width + x] = c;
                        src.c_green[y * src.width + x] = c;
                        src.c_blue[y * src.width + x] = c;
                }
        }

        if (png_8rgb_write(f, &src))
        {
                printf("Failed to write\n");
        }

        if (png_f32_read(&dst, f))
        {
                printf("Failed to read\n");
        }

        printf("%d %d %d\n", dst.width, dst.height, dst.color_type);

        for (unsigned int y = 0; y < dst.height; y++)
        {
                for (unsigned int x = 0; x < dst.width; x++)
                {
                        unsigned int offset = y * src.width + x;
                        unsigned int red = (int)(dst.c_red[offset] * 255.0f);
                        unsigned int green = (int)(dst.c_green[offset] * 255.0f);
                        unsigned int blue = (int)(dst.c_blue[offset] * 255.0f);

                        printf("%02d ", blue);

                        /* Compare */
                        if (red != y * src.width + x)
                        {
                                printf("ERROR\n");
                                abort();
                        }
                        if (red != blue)
                        {
                                printf("ERROR\n");
                                abort();                                
                        }
                        if (red != green)
                        {
                                printf("ERROR\n");
                                abort();                                
                        }
                        
                }
                printf("\n");
        }

        printf("Success\n");

        bm_free_8(&src);
        bm_free_f32(&dst);

        return 0;
}
