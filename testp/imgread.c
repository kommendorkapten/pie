#include <stdio.h>
#include "../pie_types.h"
#include "../bm/pie_bm.h"
#include "../io/pie_io.h"

int main(int argc, char** argv)
{
        struct pie_bitmap_f32rgb bm;
        int ret;        

        if (argc != 2)
        {
                return -1;
        }

#if 0
        ret = png_f32_read(&bm, argv[1]);
#else
        ret = pie_io_load(&bm, argv[1]);
#endif
        if (ret)
        {
                printf("Could not load media: %d\n", ret);
                return ret;
        }
        printf("width:     %d\n", bm.width);
        printf("height:    %d\n", bm.height);
        printf("stride:    %d\n", bm.row_stride);
        printf("channels:  %d\n", (int)bm.color_type);
        printf("bit depth: %d\n", (int)bm.bit_depth);
        pie_bm_free_f32(&bm);
        
        return ret;
}
