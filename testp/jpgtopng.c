#include <stdio.h>
#include <stdlib.h>
#include "../pie_types.h"
#include "../pie_bm.h"
#include "../io/pie_io.h"

int main(void)
{
        char* in = "out.jpg";
        char* out = "out.png";
        struct bitmap_f32rgb bmf;
        struct bitmap_u8rgb bmu;

        if (jpg_f32_read(&bmf, in))
        {
                printf("1\n");
                return -1;
        }
        if (bm_conv_bd(&bmu, PIE_COLOR_8B,
                       &bmf, PIE_COLOR_32B))
        {
                printf("1.5");
        }
        if (png_u8rgb_write(out, &bmu))
        {
                printf("2\n");
                return -1;
        }
        
        bm_free_u8(&bmu);
        bm_free_f32(&bmf);

        return 0;
}
