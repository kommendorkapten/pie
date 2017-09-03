#include <stdlib.h>
#include <stdio.h>
#include "../pie_types.h"
#include "../bm/pie_bm.h"
#include "../io/pie_io.h"
#include "../alg/pie_cspace.h"

int main(int argc, char** argv)
{
        /* int scale; */
        int ret;
        struct pie_bitmap_f32rgb img;
        float gamma = 0.454550f;
        
        if (argc != 2)
        {
                printf("Usage: analin file\n");
                return 1;
        }
        /* scale = atoi(argv[2]); */
        
        ret = pie_io_load(&img, argv[1]);
        if (ret)
        {
                printf("Error reading file. Code %d\n", ret);
                return 1;
        }

        printf("  x lin gma\n");
        printf("-----------\n");        
        for (int x = 0; x < img.width; x+=4)
        {
                int l = (int)(img.c_red[x] * 255.0f);
                int c = (int)(pie_gamma(img.c_red[x], gamma) * 255.0f);
                printf("%03d %03d %03d\n", x/4, l, c);

                if (((x/ 4) + 1)% 24 == 0)
                {
                        printf("  x lin gma\n");
                        printf("-----------\n");
                }
        }

        pie_bm_free_f32(&img);

        return 0;
}
