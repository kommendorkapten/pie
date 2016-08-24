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
        src.pixels = malloc(src.height * src.width * sizeof(struct pixel_8rgb));
        src.color_type = PIE_COLOR_TYPE_RGB;

        for (int y = 0; y < src.height; y++)
        {
                for (int x = 0; x < src.width; x++)
                {
                        uint8_t c = y * src.width + x;

                        src.pixels[y * src.width + x].red = c;
                        src.pixels[y * src.width + x].green = c;
                        src.pixels[y * src.width + x].blue = c;
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

        for (int y = 0; y < dst.height; y++)
        {
                for (int x = 0; x < dst.width; x++)
                {
                        int offset = y * src.width + x;

                        printf("%02d ", (int)(dst.pixels[offset].blue * 255.0f));
                }
                printf("\n");
        }

        free(src.pixels);
        free(dst.pixels);

        return 0;
}
