#include <stdio.h>
#include "../pie_types.h"
#include "../pie_bm.h"
#include "../io/pie_io.h"
#include "../lib/timing.h"
#include "../alg/pie_contr.h"

int main(int argc, char** argv)
{
        int ret;
        struct bitmap_f32rgb img;
        struct timing t;
        unsigned long dur;

        if (argc != 2)
        {
                printf("Usage contr filename\n");
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
        pie_alg_contr(img.c_red,
                      0.4f,
                      img.width,
                      img.height,
                      img.row_stride);
        pie_alg_contr(img.c_green,
                      0.4f,
                      img.width,
                      img.height,
                      img.row_stride);
        pie_alg_contr(img.c_blue,
                      0.4f,
                      img.width,
                      img.height,
                      img.row_stride);

        dur = timing_dur_usec(&t);
        printf("Calculate contrast took %luusec\n", dur);

        bm_free_f32(&img);
        return 0;
}
