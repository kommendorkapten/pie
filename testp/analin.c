#include "../pie_types.h"
#include "../pie_bm.h"
#include "../io/pie_io.h"
#include "../pie_cspace.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv)
{
        int scale;
        int ret;
        struct bitmap_f32rgb img;

        if (argc != 3)
        {
                printf("Usage: analin file scalefactor\n");
                return 1;
        }
        scale = atoi(argv[2]);
        
        ret = pie_io_load(&img, argv[1]);
        if (ret)
        {
                printf("Error reading file. Code %d\n", ret);
                return 1;
        }

        /* Gamma decompress second row */
        srgb_to_linearv(img.c_red + img.row_stride, img.width);
        for (unsigned int x = 0; x < img.width; x+=4)
        {
                int l = (int)(img.c_red[x] * 255.0);
                int c = (int)(img.c_red[img.row_stride + x] * 255.0f);
                printf("%03d %03d %03d\n", x/4, l, c);
        }

        bm_free_f32(&img);

        return 0;
}
