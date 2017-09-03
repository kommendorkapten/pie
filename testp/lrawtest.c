#include <stdio.h>
#include <strings.h>
#include <libraw/libraw.h>
#include "../lib/timing.h"
#include "../pie_types.h"
#include "../bm/pie_bm.h"
#include "../io/pie_io_jpg.h"
#include "../io/pie_io_png.h"

int main(int argc, char** argv)
{
        struct timing t;
        libraw_data_t* lrd;
        int ret;

        if (argc != 2)
        {
                printf("Usage lrawtest filename\n");
                return -1;
        }
        
        lrd = libraw_init(0);
        if (lrd == NULL)
        {
                printf("Failed to init libraw\n");
                return 1;
        }

        /* output params:
         * user_mul (white balance)
         * threshold (noise reduction)
         * use_auto_wb (auto wb calculation)
         * use_camera_wb (camer wb used)
         * output_color (0 raw, 1 sRGB, 2 Adobe, 3 Wide, 4 ProPhoto, 5 XYZ)
         * output_bps (8 or 16)
         * output_tiff 0/1: output PPM/TIFF.
         * user_qual 0 - linear interpolation
                     1 - VNG interpolation
                     2 - PPG interpolation
                     3 - AHD interpolation
                     4 - DCB interpolation
         * user_sat (saturation adj)
         * dcb_iterations num dcp passes (def -1, i.e none)
         * dcb_enhance_fl nonzero turn on enhanced interpolation
         * fbdd_noserd 0 - do not use FBDD noise reduction
                       1 - light FBDD reduction
                       2 (and more) - full FBDD reduction
         */
        lrd->params.output_bps = 16;
        lrd->params.use_camera_wb = 1;
        lrd->params.user_flip = 0;
        lrd->params.output_color = 1; /* sRGB */

        //lrd->params.user_qual = -1; // unpack 2.3, process 13.7
        lrd->params.user_qual = 0; // unpack 2.4, process 3.8
        //lrd->params.user_qual = 1; // unpack 2.4 process 20.2
        //lrd->params.user_qual = 2; // unpack 2.4 process 4.7 This is OpenMP supported
        //lrd->params.user_qual = 3; // unpack 2.4 process 14.0 This is OpenMp supported
        //lrd->params.user_qual = 4; // unpack 2.4 process 16.0
        
        printf("threshold %f\n", lrd->params.threshold);
        printf("auto wb %d\n", lrd->params.use_auto_wb);
        printf("camera wb %d\n", lrd->params.use_camera_wb);
        printf("output color %d\n", lrd->params.output_color);
        printf("output bps %d\n", lrd->params.output_bps);
        printf("output tiff %d\n", lrd->params.output_tiff);
        printf("user qual %d\n", lrd->params.user_qual);
        printf("user sat %d\n", lrd->params.user_sat);
        printf("dcb iterations %d\n", lrd->params.dcb_iterations);
        printf("dbc enhance %d\n", lrd->params.dcb_enhance_fl);
        printf("fbdd noiserd %d\n", lrd->params.fbdd_noiserd);
        
        /* Inspect params, progress_flags */

        timing_start(&t);
        ret = libraw_open_file(lrd, argv[1]);
        if (ret)
        {
                printf("libraw_open_file:failed %d\n", ret);
                return 1;
        }
        printf("* libraw_open_file %0.3fs\n", timing_dur_usec(&t) / 1000000.0f);

#if 0
        printf("Make:  %s\n", lrd->idata.make);
        printf("Model: %s\n", lrd->idata.model);
        printf("Size: %d x %d\n", lrd->sizes.width, lrd->sizes.height);
        switch (lrd->sizes.flip)
        {
        case 0:
                lrd->sizes.flip = 1;
                break;
        case 5:
                lrd->sizes.flip = 8;
                break;
        }
        printf("Flip: %d\n", lrd->sizes.flip);
        /*
         * Flip (fuji) (canon) Exif
         * 0            0      1    (img 0)
         * 5            5      8    (img 90)
         * 3            0      3    (img 180)
         * 6            6      6    (img 270)
         */

        printf("ISO: %.f\n", lrd->other.iso_speed);
        printf("Aperture: %.f\n", lrd->other.aperture);
        printf("Time: %ld\n", lrd->other.timestamp);
        if (lrd->other.shutter > 0.0f && lrd->other.shutter < 1.0f)
        {
                printf("Shutter speed: 1/%.fs\n", 1 / lrd->other.shutter);
        }
        else
        {
                printf("Shutter speed: %0.1fs\n", lrd->other.shutter);
        }
        printf("Focal length: %.f\n", lrd->other.focal_len);
        printf("Artist: %s\n", lrd->other.artist);

        if (strcasecmp(lrd->idata.make, "canon") == 0)
        {
                printf("Flash mode %d\n", lrd->makernotes.canon.FlashMode);
        }
        else if (strcasecmp(lrd->idata.make, "fujifilm") == 0)
        {
                printf("Flash mode %d\n", lrd->makernotes.fuji.FlashMode);
        }
        printf("Lens %s\n", lrd->lens.Lens);
        printf("Metering mode %d\n", lrd->shootinginfo.MeteringMode);
        printf("Exposure mode %d\n", lrd->shootinginfo.ExposureMode);
#endif
/*
//	char           *ped_artist;
	char           *ped_copyright;
	char           *ped_software;
//	char           *ped_date_time;
//	char           *ped_lens_model;
//	char           *ped_make;
//	char           *ped_model;
//	char           *ped_exposure_time;
	int             ped_sub_sec_time;
//	int             ped_x_dim;
//	int             ped_y_dim;
//	int             ped_iso;
	int             ped_gamma;
	int             ped_white_point;
// 	short           ped_orientation;
//	short           ped_focal_len;
//	short           ped_fnumber;
	short           ped_exposure_bias;
	short           ped_white_balance;
	short           ped_exposure_prog;
//	short           ped_metering_mode;
	short           ped_flash;
//	short           ped_exposure_mode;
//	short           ped_color_space;
*/

        timing_start(&t);
        if (libraw_unpack(lrd))
        {
                printf("Unpack failed\n");
                return 1;
        }
        printf("* libraw_unpack %0.3fs\n", timing_dur_usec(&t) / 1000000.0f);

        timing_start(&t);
        if (libraw_dcraw_process(lrd))
        {
                printf("dcraw process failed\n");
                return 1;
        }
        printf("* libraw_dcraw_process %0.3fs\n", timing_dur_usec(&t) / 1000000.0f);

        timing_start(&t);
        libraw_processed_image_t* mem_img;
        mem_img = libraw_dcraw_make_mem_image(lrd, &ret);
        if (mem_img == NULL)
        {
                printf("dcraw make mem failed\n");
                return 1;                
        }
        printf("* libraw_dcraw_make_mem_image %0.3fs\n", timing_dur_usec(&t) / 1000000.0f);
        printf("Type: %d (1 jpeg, 2 bitmap)\n", mem_img->type);
        printf("Dim: %d x %d\n", mem_img->width, mem_img->height);
        printf("Colors: %d\n", mem_img->colors);
        printf("Bits: %d\n", mem_img->bits);
        printf("size: %dKB\n", mem_img->data_size / 1024);

        struct pie_bitmap_f32rgb bm;
        struct pie_bitmap_u16rgb out;
        bm.width = mem_img->width;
        bm.height = mem_img->height;
        bm.color_type = PIE_COLOR_TYPE_RGB;
        pie_bm_alloc_f32(&bm);
        printf("bm width: %d\n", bm.width);
        printf("bm stride: %d\n", bm.row_stride);

        /* Copy image */
        timing_start(&t);
        unsigned char* data = mem_img->data;
        int pixel_step = mem_img->bits / 8;
        for (int y = 0; y < bm.height; y++)
        {
                for (int x = 0; x < bm.width; x++)
                {
                        int p = y * bm.row_stride + x;

                        bm.c_red[p] = ((float)*(unsigned short*)data) / 65535.0f;
                        data += pixel_step;
                        
                        bm.c_green[p] = ((float)*(unsigned short*)data) / 65535.0f;
                        data += pixel_step;
                        
                        bm.c_blue[p] = ((float)*(unsigned short*)data) / 65535.0f;
                        data += pixel_step;
                }
        }
        printf("* copy image to pie bm %0.3fs\n", timing_dur_usec(&t) / 1000000.0f);
        
        timing_start(&t);
        libraw_dcraw_clear_mem(mem_img);
        printf("* libraw_dcraw_clear_mem %0.3fs\n", timing_dur_usec(&t) / 1000000.0f);

        timing_start(&t);
        libraw_close(lrd);
        printf("* libraw_close %0.3fs\n", timing_dur_usec(&t) / 1000000.0f);

        timing_start(&t);
        pie_bm_conv_bd(&out,
                       PIE_COLOR_16B,
                       &bm,
                       PIE_COLOR_32B);
        printf("* pie_bm_conv %0.3fs\n", timing_dur_usec(&t) / 1000000.0f);

        timing_start(&t);
        if (png_u16rgb_write("out.png", &out))
        {
                printf("write jpg failed\n");
                return 1;
        }
        printf("* jpg_u8rgb_write %0.3fs\n", timing_dur_usec(&t) / 1000000.0f);
        
        pie_bm_free_f32(&bm);
        pie_bm_free_u16(&out);
        return 0;
}

/* Comment out libraw_progress_treserved3 (1<<31)*/

/*

user_qual = 0
./bin/lrawtest ~/Documents/Pictures/pie/incoming_raw/FSNF0172.RAF
* libraw_open_file 0.002s
* libraw_unpack 0.318s
* libraw_dcraw_process 5.309s
* libraw_dcraw_make_mem_image 0.511s
* copy image to pie bm 0.458s
* libraw_dcraw_clear_mem 0.000s
* libraw_close 0.000s
* pie_bm_conv 0.205s
* jpg_u8rgb_write 1.550s

./bin/lrawtest ~/Documents/Pictures/pie/incoming_raw/FSN_3000.CR2 
* libraw_open_file 0.006s
* libraw_unpack 2.362s
* libraw_dcraw_process 3.846s
* libraw_dcraw_make_mem_image 0.373s
* copy image to pie bm 0.334s
* libraw_dcraw_clear_mem 0.000s
* libraw_close 0.000s
* pie_bm_conv 0.150s
* jpg_u8rgb_write 1.021s

user_qual = 2 
./bin/lrawtest ~/Documents/Pictures/pie/incoming_raw/FSNF0172.RAF
* libraw_open_file 0.005s
* libraw_unpack 0.420s
* libraw_dcraw_process 44.925s
* libraw_dcraw_make_mem_image 0.503s
* copy image to pie bm 0.457s
* libraw_dcraw_clear_mem 0.000s
* libraw_close 0.000s
* pie_bm_conv 0.199s
* jpg_u8rgb_write 1.461s

./bin/lrawtest ~/Documents/Pictures/pie/incoming_raw/FSN_3000.CR2 
* libraw_open_file 0.005s
* libraw_unpack 2.371s
* libraw_dcraw_process 4.739s
* libraw_dcraw_make_mem_image 0.382s
* copy image to pie bm 0.340s
* libraw_dcraw_clear_mem 0.000s
* libraw_close 0.000s
* pie_bm_conv 0.150s
* jpg_u8rgb_write 1.059s

openmp: No diff
 */
