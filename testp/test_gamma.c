#include <stdio.h>
#include <stdlib.h>
#include "../alg/pie_cspace.h"
#include "../alg/pie_contr.h"

#define STD_GAMMA_EXPAND 2.2f
#define STD_GAMMA_COMPRESS (1.0f/STD_GAMMA_EXPAND)

/*
  File formats should be stored using sRGB.
  i.e write data in sRGB format, and pixels read are in sRGB.
 */

int main(void)
{
        float gam[] =   {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.723f, 0.8f, 0.9f, 1.0f};
        float lin[] =   {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.723f, 0.8f, 0.9f, 1.0f};
        float gam_c[] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.723f, 0.8f, 0.9f, 1.0f};
        float lin_c[] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.723f, 0.8f, 0.9f, 1.0f};
        int len = sizeof(gam) / sizeof(gam[0]);

        /* To linear */
        for (int i = 0; i < len; i++)
        {
                lin[i] = pie_alg_srgb_to_linear(lin[i]);
                lin_c[i] = pie_alg_srgb_to_linear(lin_c[i]);
        }

        pie_alg_contr(gam_c, 1.1f, len, 1, 0);
        pie_alg_contr(lin_c, 1.1f, len, 1, 0);

        /* Done */
        printf("Gamma    linear   contrast-g contrast-l contrast-l-g\n");
        for (int i = 0; i < len; i++)
        {
                printf("%f %f %f   %f %f\n",
                       gam[i],
                       lin[i],
                       gam_c[i],
                       lin_c[i],
                       pie_alg_linear_to_srgb(lin_c[i]));
        }

        return 0;
}
