#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <nmmintrin.h> /* sse 4.2 */
#include "timing.h"

#if 1
#define WIDTH 5184
#define HEIGHT 3456
#else
#define WIDTH 1027
#define HEIGHT 682
#endif
#define LUM_RED 0.2126f
#define LUM_GREEN 0.7152f
#define LUM_BLUE 0.0722f

struct bm
{
	float* red;
	float* green;
	float* blue;
	unsigned int width;
	unsigned int height;
	unsigned int row_stride;
};

int hist_n[256];
int hist_s[256];

float lum_redv[4] = {LUM_RED, LUM_RED, LUM_RED, LUM_RED};
float lum_greenv[4] = {LUM_GREEN, LUM_GREEN, LUM_GREEN, LUM_GREEN};
float lum_bluev[4] = {LUM_BLUE, LUM_BLUE, LUM_BLUE, LUM_BLUE};
float vec_255[4] = {255.0f, 255.0f, 255.0f, 255.0f};

/*
  -O2 -mtune=native -> naive ~55ms
  -02 -mtune=native -> sse   ~40ms
 */

void calc_hist(struct bm*);
void calc_hist_sse(struct bm*);

int main(void)
{
	size_t len;
	size_t nump = WIDTH * HEIGHT;
	struct bm bm18mp;
	struct timing t;
	unsigned long dur;
	size_t count = 0;
	unsigned rem = WIDTH % 4;

	if (rem == 0)
	{
		bm18mp.row_stride = WIDTH;
	}
	else
	{
		bm18mp.row_stride = WIDTH + (4 - rem);
	}
	
	for (int i = 0; i < 256; i++)
	{
		hist_n[i] = 0;
		hist_s[i] = 0;
	}
	
	bm18mp.width = WIDTH;
	bm18mp.height = HEIGHT;
	len = bm18mp.row_stride * HEIGHT;

	bm18mp.red = malloc(len * sizeof(float));
	bm18mp.green = malloc(len * sizeof(float));
	bm18mp.blue = malloc(len * sizeof(float));

	for (size_t i = 0; i < len; i++)
	{
		size_t j;

		j = rand() %  256;
		bm18mp.red[i] = j / 255.0f;

		j = rand() %  256;
		bm18mp.green[i] = j / 255.0f;

		j = rand() %  256;
		bm18mp.blue[i] = j / 255.0f;		
		
	}
	
	memset(hist_n, 0, 256 * sizeof(int));
	timing_start(&t);
	calc_hist(&bm18mp);
	dur = timing_dur_usec(&t);
	printf("hist naive in %luusec\n", dur);

	memset(hist_n, 0, 256 * sizeof(int));
	timing_start(&t);
	calc_hist(&bm18mp);
	dur = timing_dur_usec(&t);
	printf("hist naive in %luusec\n", dur);

	memset(hist_n, 0, 256 * sizeof(int));	
	timing_start(&t);
	calc_hist(&bm18mp);
	dur = timing_dur_usec(&t);
	printf("hist naive in %luusec\n", dur);

	memset(hist_s, 0, 256 * sizeof(int));	
	timing_start(&t);
	calc_hist_sse(&bm18mp);
	dur = timing_dur_usec(&t);
	printf("hist sse in %luusec\n", dur);

	memset(hist_s, 0, 256 * sizeof(int));
	timing_start(&t);
	calc_hist_sse(&bm18mp);
	dur = timing_dur_usec(&t);
	printf("hist sse in %luusec\n", dur);

	memset(hist_s, 0, 256 * sizeof(int));	
	timing_start(&t);
	calc_hist_sse(&bm18mp);
	dur = timing_dur_usec(&t);
	printf("hist sse in %luusec\n", dur);	

	count = 0;
	for (int i = 0; i < 256; i++)
	{
		count += hist_n[i];
		if (hist_n[i] != hist_s[i])
		{
			printf("Diff at %d, %d vs %d\n",
			       i,
			       hist_n[i],
			       hist_s[i]);
		}
	}
	printf("\n");

	printf("%lu\n", count);
	printf("%lu\n", nump);

	if (count != nump)
	{
		printf("DIFF in num pixels\n");
	}
	
	free(bm18mp.red);
	free(bm18mp.green);
	free(bm18mp.blue);
}

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
