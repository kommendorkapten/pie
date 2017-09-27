#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../lib/timing.h"
#include "../io/pie_io.h"
#include "../pie_types.h"
#include "../bm/pie_bm.h"
#include "../bm/pie_bm_dwn_smpl.h"

#define MAX_PATH 256
#define OUTPUT_16B 0

int main(int argc, char** argv)
{
        struct pie_bitmap_f32rgb img;
        struct pie_bitmap_f32rgb dwn;        
#if OUTPUT_16B
        struct pie_bitmap_u16rgb out;
#else
        struct pie_bitmap_u8rgb out;
#endif
        struct timing t;
        char fout[MAX_PATH] = {0};
        char fin[MAX_PATH] = {0};
        time_t dur;
        int c;        
        int max = 0;
        int ret;

        while ((c = getopt(argc, argv, "f:o:d:")) != -1)
        {
                switch (c)
                {
                case 'f':
                        strncpy(fin, optarg, MAX_PATH);
                        fin[MAX_PATH - 1] = 0;
                        break;
                case 'o':
                        strncpy(fout, optarg, MAX_PATH);
                        fin[MAX_PATH - 1] = 0;                        
                        break;
                case 'd':
                        max = atoi(optarg);
                        break;
                }
        }

        if (strlen(fin) == 0)
        {
                printf("No input file provided\n");
                return 1;
        }

        if (strlen(fout) == 0)
        {
                printf("No output file provided\n");
                return 1;
        }

        if (max < 1)
        {
                printf("Invalid output dimension\n");
                return 1;
        }
        
        timing_start(&t);
        ret = pie_io_load(&img, fin, NULL);
        dur = timing_dur_usec(&t);
        if (ret)
        {
                printf("Error loading media: %d\n", ret);
                return -1;
        }
        printf("Loaded media in %luusec\n", dur);
        
        timing_start(&t);

        if (pie_bm_dwn_smpl(&dwn, &img, max, max))
        {
                abort();
        }
        
        dur = timing_dur_usec(&t);
        printf("Downsample took %luusec\n", dur);

        timing_start(&t);        
        pie_bm_conv_bd(&out,
#if OUTPUT_16B
                       PIE_COLOR_16B,
#else
                       PIE_COLOR_8B,
#endif
                       &dwn,
                       PIE_COLOR_32B);
        dur = timing_dur_usec(&t);
        printf("To 16bit took %luusec\n", dur);

        timing_start(&t);        
#if OUTPUT_16B
        pie_io_png_u16rgb_write(fout, &out);
#else
        pie_io_png_u8rgb_write(fout, &out);
#endif
        dur = timing_dur_usec(&t);
        printf("Write 16bit PNG took %luusec\n", dur);

        pie_bm_free_f32(&img);
        pie_bm_free_f32(&dwn);        
#if OUTPUT_16B
        pie_bm_free_u16(&out);
#else
        pie_bm_free_u8(&out);
#endif
        return 0;
}
