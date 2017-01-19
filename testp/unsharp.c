#include <stdio.h>
#include <math.h>
#include "../pie_types.h"
#include "../bm/pie_bm.h"
#include "../io/pie_io.h"
#include "../lib/timing.h"
#include "../alg/pie_unsharp.h"

int main(int argc, char** argv)
{
        int ret;
        struct bitmap_f32rgb img;
        struct bitmap_u8rgb out;
        struct timing t;
        struct pie_unsharp_param p;
        unsigned long dur;
        
        p.radius = 50.0;
        p.amount = 0.125;    /* 0.3 to 0.7 is suitable values */
        p.threshold = 2.0f; /* typical 3 to 20 */

        if (argc != 2)
        {
                printf("Usage unsharp filename\n");
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

        ret = pie_unsharp(&img, &p);
        if (ret)
        {
                abort();
        }
        
        dur = timing_dur_usec(&t);
        printf("Executed unsharp mask took %luusec\n", dur);

        bm_conv_bd(&out, PIE_COLOR_8B,
                   &img, PIE_COLOR_32B);

        png_u8rgb_write("out.png", &out);

        bm_free_f32(&img);
        bm_free_u8(&out);
        
        return 0;
}
