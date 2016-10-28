#include <stdio.h>
#include "../pie_types.h"
#include "../pie_bm.h"
#include "../io/pie_io.h"
#include "../lib/timing.h"
#include "../alg/pie_hist.h"

int main(int argc, char** argv)
{
        int ret;
        struct pie_histogram h;
        struct bitmap_f32rgb img;
        struct timing t;
        unsigned long dur;
        unsigned int count = 0;

        if (argc != 2)
        {
                printf("Usage histinfo filename\n");
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
        pie_alg_hist_lum(&h, &img);
        dur = timing_dur_usec(&t);
        printf("Calculate historgram took %luusec\n", dur);

        for (int i = 0; i < 256; i++)
        {
                count += h.lum[i];
                printf("%03d % 7u\n", i, h.lum[i]);
        }

        if (count != img.width * img.height)
        {
                printf("Counted %u pixels, expected is %u\n", 
                       count, img.width * img.height);
        }
        bm_free_f32(&img);
        return 0;
}
