#include <stdio.h>
#include <stdlib.h>
#include "../pie_types.h"
#include "../pie_bm.h"
#include "../io/pie_io.h"
#include "../lib/timing.h"

int main(int argc, char** argv)
{
        char* in;
        char* out;
        struct bitmap_f32rgb bmf;
        struct bitmap_u8rgb bmu;
        struct timing t;

        if (argc != 3)
        {
                printf("Usage: jpgtong infile outfile\n");
                return 1;
        }

        in = argv[1];
        out = argv[2];

        timing_start(&t);
        if (jpg_f32_read(&bmf, in))
        {
                printf("1\n");
                return -1;
        }
        printf("Read %s in %luusec\n", in, timing_dur_usec(&t));

        timing_start(&t);
        if (bm_conv_bd(&bmu, PIE_COLOR_8B,
                       &bmf, PIE_COLOR_32B))
        {
                printf("1.5");
        }
        printf("Converted img to u8 in %luusec\n", timing_dur_usec(&t));

        timing_start(&t);
        if (png_u8rgb_write(out, &bmu))
        {
                printf("2\n");
                return -1;
        }
        printf("Wrote %s in %luusec\n", out, timing_dur_usec(&t));        

        bm_free_u8(&bmu);
        bm_free_f32(&bmf);

        return 0;
}
