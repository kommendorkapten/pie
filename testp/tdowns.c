#include <stdio.h>
#include "../lib/timing.h"
#include "../io/pie_io.h"
#include "../pie_types.h"
#include "../bm/pie_bm.h"
#include "../bm/pie_dwn_smpl.h"

int main(int argc, char** argv)
{
        int ret;
        struct bitmap_f32rgb img;
        struct bitmap_f32rgb dwn;        
        struct bitmap_u8rgb out;
        struct timing t;
        suseconds_t dur;
        
        if (argc != 2)
        {
                printf("Usage tdowns filename\n");
                return -1;
        }
        
        timing_start(&t);
        ret = pie_io_load(&img, argv[1]);
        dur = timing_dur_usec(&t);
        if (ret)
        {
                printf("Error loading media: %d\n", ret);
                return -1;
        }
        printf("Loaded media in %luusec\n", dur);
        
        timing_start(&t);

        if (pie_dwn_smpl(&dwn, &img, 960, 960))
        {
                abort();
        }
        
        dur = timing_dur_usec(&t);
        printf("Downsample took %luusec\n", dur);

        bm_conv_bd(&out, PIE_COLOR_8B,
                   &dwn, PIE_COLOR_32B);

        png_u8rgb_write("down.png", &out);

        bm_free_f32(&img);
        bm_free_f32(&dwn);        
        bm_free_u8(&out);
        return 0;
}
