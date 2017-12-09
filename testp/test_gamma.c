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
        float ov[] = {0.1f, 0.2f, 0.4f, 0.5f, 0.6f, 0.7f, 0.723f, 0.8f, 0.9f, 1.0f};
        float v[] = {0.1f, 0.2f, 0.4f, 0.5f, 0.6f, 0.7f, 0.723f, 0.8f, 0.9f, 1.0f};
        float gcv[] = {0.1f, 0.2f, 0.4f, 0.5f, 0.6f, 0.7f, 0.723f, 0.8f, 0.9f, 1.0f};
        float ogcv[] = {0.1f, 0.2f, 0.4f, 0.5f, 0.6f, 0.7f, 0.723f, 0.8f, 0.9f, 1.0f};
        int len = sizeof(v) / sizeof(v[0]);

        /* To linear */
        for (int i = 0; i < len; i++)
        {
                gcv[i] = srgb_to_linear(gcv[i]);
        }

#if 0
        /* Apply alg */
        for (int i = 0; i < len; i++)
        {
                v[i] = v[i] * 1.1;
                gcv[i] = gcv[i] * 1.1;
        }
#endif

        pie_alg_contr(v, 1.1f, len, 1, 0);
        pie_alg_contr(gcv, 1.1f, len, 1, 0);

        /* To sRGB */
        for (int i = 0; i < len; i++)
        {
                gcv[i] = linear_to_srgb(gcv[i]);
        }

        /* Done */
        for (int i = 0; i < len; i++)
        {
                printf("%f %f %f %f\n", ov[i], v[i], ogcv[i], gcv[i]);
        }

        return 0;
}
