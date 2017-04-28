#include <stdio.h>
#include <stdlib.h>
#include "../pie_types.h"
#include "../bm/pie_bm.h"
#include "../io/pie_io.h"

int main(void)
{
        char* f = "out.png";
        struct pie_bitmap_u8rgb src;
        struct pie_bitmap_f32rgb dst;
        
        src.height = 10;
        src.width = 10;
        src.color_type = PIE_COLOR_TYPE_RGB;
        pie_bm_alloc_u8(&src);

        for (int y = 0; y < src.height; y++)
        {
                for (int x = 0; x < src.width; x++)
                {
                        uint8_t c = (uint8_t)(y * src.width + x);
                        int o = y * src.row_stride + x;

                        src.c_red[o] = c;
                        src.c_green[o] = c;
                        src.c_blue[o] = c;
                }
        }

        if (png_u8rgb_write(f, &src))
        {
                printf("Failed to write\n");
        }

        if (png_f32_read(&dst, f))
        {
                printf("Failed to read\n");
        }

        printf("%d %d %d\n", dst.width, dst.height, (int)dst.color_type);

        for (int y = 0; y < dst.height; y++)
        {
                for (int x = 0; x < dst.width; x++)
                {
                        int offset = y * src.row_stride + x;
                        int red = (int)(dst.c_red[offset] * 255.0f);
                        int green = (int)(dst.c_green[offset] * 255.0f);
                        int blue = (int)(dst.c_blue[offset] * 255.0f);

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

        pie_bm_free_u8(&src);
        pie_bm_free_f32(&dst);

        return 0;
}
