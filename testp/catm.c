#include "../math/pie_catmull.h"
#include "../alg/pie_expos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_P 150

int main(void)
{
        struct pie_point_2d p[5];
        struct pie_point_2d o[2 * NUM_P];
        int out[256];

        p[0].x = -0.4f;
        p[0].y = -1.0f;
        p[1].x = 0.0f;
        p[1].y = 0.0f;
        p[2].x = 0.55f;
        p[2].y = 0.75f;
        p[3].x = 1.005;
        p[3].y = 1.005;
        p[4].x = 2.0f;
        p[4].y = 1.2f;

        /* test */
        pie_alg_expos_curve(p, 0.0);        
        pie_catm_rom_chain(o, p, 5, NUM_P);

#if 1
        int last = 0;
        for (int i = 0; i < 2 * NUM_P; i++)
        {
                printf("%f %f %3d %3d\n", o[i].x, o[i].y, (int)(o[i].x*255.0), (int)(o[i].y * 255));
                if ((last - (int)(o[i].x*255.0)) != -1)
                {
                        printf("*** ***\n");
                }
                last = (int)(o[i].x*255.0);
        }
#endif

        memset(out, -1, 256 * sizeof(int));
        
        /* Merge */
        for (int i = 0; i < (2 * NUM_P); i ++)
        {
                int x = (int)(o[i].x * 255.0f);
                int y = (int)(o[i].y * 255.0f);

                if (x > 255)
                {
                        printf("%d@%d (%f)\n", x, i, o[i].x);
                        abort();
                }

                out[x] = y;
        }
#if 0
        for (int i = 0; i < 256; i++)
        {
                printf("%3d\n", out[i]);
        }
#endif

        pie_alg_expos_curve(p, -0.1);
        for (int i = 0; i < 5; i++)
        {
                printf("%f %f\n", p[i].x, p[i].y);
        }
        
        return 0;
}
