#include <stdio.h>
#include <stdlib.h>
#include "pie_cspace.h"

#define STD_GAMMA_EXPAND 2.2f
#define STD_GAMMA_COMPRESS (1.0f/STD_GAMMA_EXPAND)

int main(void)
{
        float* f = malloc(sizeof(float) * 256);

        for (int i = 0; i < 256; i++)
        {
                f[i] = i / 255.0f;
        }

//        gammav(f, STD_GAMMA_EXPAND, 256);

        for (int i = 0; i < 256; i++)
        {
                f[i] *= 2.0f * gamma(f[i], STD_GAMMA_EXPAND);
        }

//        gammav(f, STD_GAMMA_COMPRESS, 256);
        for (int i = 0; i < 256; i++)
        {
                float in = i / 255.0f;
                printf("In %.3f Out %.3f Delta %.7f%%\n", in, f[i], (f[i] - in)/in);
        }

        return 0;
}
