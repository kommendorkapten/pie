/* Render function for a single channel */
typedef (*pie_render_f)(float* out, 
                        float* in, 
                        unsigned int width, 
                        unsigned int height, 
                        unsigned int stride);


float contrast_brightness(float in,
                          float c, /* [0:+] */
                          float b) /* [-1.0:1.0] */
{
        /* 
           c [0, 1] less contrast
           c [1,+] more contrast
           b is pure brightness
        */
        return c(in - 0.5f) + 0.5f + b;
}

#define  Pr  .299
#define  Pg  .587
#define  Pb  .114



//  public-domain function by Darel Rex Finley
//
//  The passed-in RGB values can be on any desired scale, such as 0 to
//  to 1, or 0 to 255.  (But use the same scale for all three!)
//
//  The "change" parameter works like this:
//    0.0 creates a black-and-white image.
//    0.5 reduces the color saturation by half.
//    1.0 causes no change.
//    2.0 doubles the color saturation.
//  Note:  A "change" value greater than 1.0 may project your RGB values
//  beyond their normal range, in which case you probably should truncate
//  them to the desired range before trying to use them in an image.

void changeSaturation(double *R, double *G, double *B, double change) {

  double  P=sqrt(
  (*R)*(*R)*Pr+
  (*G)*(*G)*Pg+
  (*B)*(*B)*Pb ) ;

  *R=P+((*R)-P)*change;
  *G=P+((*G)-P)*change;
  *B=P+((*B)-P)*change; }

int hist_s[256];

float lum_redv[4] = {LUM_RED, LUM_RED, LUM_RED, LUM_RED};
float lum_greenv[4] = {LUM_GREEN, LUM_GREEN, LUM_GREEN, LUM_GREEN};
float lum_bluev[4] = {LUM_BLUE, LUM_BLUE, LUM_BLUE, LUM_BLUE};
float vec_255[4] = {255.0f, 255.0f, 255.0f, 255.0f};

void calc_hist(struct bm* bm)
{
        size_t count = 0;
        for (unsigned int y = 0; y < bm->height; y++)
        {
                for (unsigned int x = 0; x < bm->width; x++)
                {
                        float l;
                        unsigned int p = y * bm->row_stride + x;
                        unsigned char hp;

                        l = LUM_RED * bm->red[p];
                        l += LUM_GREEN * bm->green[p];
                        l += LUM_BLUE * bm->blue[p];

#if 0
                        if (l > 1.0f)
                        {
                                l = 1.0f;
                        }
#endif
                        
                        hp = (unsigned char)(l * 255.0f);

                        hist_n[hp]++;
                        count++;
                }
        }

        printf("Naive count: %lu\n", count);
}

void calc_hist_sse(struct bm* bm)
{
        __m128 coeff_red = _mm_load_ps(lum_redv);
        __m128 coeff_green = _mm_load_ps(lum_greenv);
        __m128 coeff_blue = _mm_load_ps(lum_bluev);
        __m128 coeff_scale = _mm_load_ps(vec_255);
        __m128 red;
        __m128 green;
        __m128 blue;
        float out[4];
        unsigned int rem = bm->width % 4;
        unsigned int stop = bm->width - rem;
        size_t count = 0;

        for (unsigned int y = 0; y < bm->height; y++)
        {
                for (unsigned int x = 0; x < stop; x+=4)
                {
                        unsigned int p = y * bm->row_stride + x;

                        red = _mm_load_ps(&bm->red[p]);
                        green = _mm_load_ps(&bm->green[p]);
                        blue = _mm_load_ps(&bm->blue[p]);

                        red = _mm_mul_ps(red, coeff_red);
                        green = _mm_mul_ps(green, coeff_green);
                        blue = _mm_mul_ps(blue, coeff_blue);
                        red = _mm_add_ps(red, green);
                        red = _mm_add_ps(red, blue);
                        red = _mm_mul_ps(red, coeff_scale);

                        _mm_store_ps(out, red);

                        hist_s[(unsigned char)out[0]]++;
                        hist_s[(unsigned char)out[1]]++;
                        hist_s[(unsigned char)out[2]]++;
                        hist_s[(unsigned char)out[3]]++;

                        count += 4;
                }

                for (unsigned int x = stop; x < bm->width; x++)
                {
                        unsigned int p = y * bm->row_stride + x;
                        unsigned char hp;
                        float l;

                        l = LUM_RED * bm->red[p];
                        l += LUM_GREEN * bm->green[p];
                        l += LUM_BLUE * bm->blue[p];

                        hp = (unsigned char)(l * 255.0f);

                        hist_s[hp]++;
                        count++;
                }
        }
        printf("sse count: %lu\n", count);      
}
