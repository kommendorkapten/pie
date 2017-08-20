#include <stdio.h>
#include "../pie_types.h"
#include "../bm/pie_bm.h"
#include "../io/pie_io.h"
#include "../lib/timing.h"

int main(int argc, char** argv)
{
        struct pie_bitmap_f32rgb bm;
        struct timing t;
        int ret;        

        if (argc != 2)
        {
                return -1;
        }

        timing_start(&t);
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
        printf("Loaded %s in %0.3fs\n",
               argv[1],
               (float)timing_dur_msec(&t) / 1000.0f);
        printf("width:     %d\n", bm.width);
        printf("height:    %d\n", bm.height);
        printf("stride:    %d\n", bm.row_stride);
        printf("channels:  %d\n", (int)bm.color_type);
        printf("bit depth: %d\n", (int)bm.bit_depth);
        pie_bm_free_f32(&bm);
        
        return ret;
}
