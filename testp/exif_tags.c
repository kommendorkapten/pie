#include <stdio.h>
#include <string.h>
#include <libexif/exif-data.h>
#include <math.h>
#include <stdint.h>

#define PIE_EXIF_TAG_LENS_1 0x0095
#define PIE_EXIF_TAG_LENS_2 0xa434

static uint16_t swap_uint16(uint16_t v)
{
        return (uint16_t)((0xff00 & v) >> 8 |
                          (0x00ff & v) << 8);

}

static uint32_t swap_uint32(uint32_t v)
{
        return (uint32_t)((0xff000000 & v) >> 24 |
                          (0x00ff0000 & v) >> 8 |
                          (0x0000ff00 & v) << 8 |
                          (0x000000ff & v) << 24);
}

static uint16_t load_exif_uint16(uint16_t v)
{
#ifdef __BYTE_ORDER__
# if __BYTE_ORDER__ == 4321
        return swap_uint16(v);
# else
        return v;
# endif
#else
#  error __BYTE_ORDER__ not defined
#endif
}

static uint32_t load_exif_uint32(uint32_t v)
{
#ifdef __BYTE_ORDER__
# if __BYTE_ORDER__ == 4321
        return swap_uint32(v);
# else
        return v;
# endif
#else
#  error __BYTE_ORDER__ not defined
#endif
}

static void trim_spaces(char *buf)
{
        char *s = buf-1;
        for (; *buf; ++buf) {
 	        if (*buf != ' ')
                        s = buf;
        }
        *++s = 0; /* nul terminate the string on the first of the final spaces */
}

/* format: 1 byte
           2 ascii
           3 short
           4 long
           5 rational
           10 srational */
/* size is including null byte for ascii */
static void show_tag(ExifData *d, ExifIfd ifd, ExifTag tag)
{
        /* See if this tag exists */
        /*ExifEntry *entry = exif_content_get_entry(d->ifd[ifd],tag);*/
        ExifEntry *entry = exif_data_get_entry(d, tag);
        if (entry) {
 	        char buf[1024];

 	        /* Get the contents of the tag in human-readable form */
 	        exif_entry_get_value(entry, buf, sizeof(buf));
 	
 	        /* Don't bother printing it if it's entirely blank */
 	        trim_spaces(buf);
 	        if (*buf) {
                        printf("% 17s: % 44s  %02d %03d\n",
                               exif_tag_get_name_in_ifd(tag,ifd),
                               buf,
                               entry->format,
                               entry->size);
 	        } else {
                        printf("nval: %s %04x\n", exif_tag_get_name_in_ifd(tag,ifd), tag);
                }

                short sval;
                long lval;
                int ival;
                int den;
                int num;

                switch (entry->format)
                {
                case EXIF_FORMAT_ASCII:
                        break;
                case EXIF_FORMAT_SHORT:
                        printf("Copy %d bytes\n", entry->size);
                        printf("Components %lu\n", entry->components);
                        memcpy(&sval, entry->data, entry->size);
                        printf("Value is: %d\n", sval);
                        printf("Swapped: %d\n", swap_uint16(sval));
                        printf("sval: %d\n", load_exif_uint16(sval));
                        break;
                case EXIF_FORMAT_LONG:
                        printf("Copy %d bytes\n", entry->size);
                        printf("Components %lu\n", entry->components);
                        memcpy(&lval, entry->data, entry->size);
                        printf("Value is: %d\n", lval);
                        break;
                case EXIF_FORMAT_RATIONAL:
                case EXIF_FORMAT_SRATIONAL:
                        memcpy(&ival, entry->data, sizeof(ival));
                        printf("irval-1: %d\n", load_exif_uint32(ival));
                        memcpy(&ival, entry->data + sizeof(ival), sizeof(ival));
                        printf("irval-2: %d\n", load_exif_uint32(ival));

                        break;
                default:
                        printf("Unknown type: %d\n", entry->format);
                }
                
        } else {
                printf("nentry: %04x\n", tag);
        }
        //printf("------------------------------------------------------------------------\n");
}

int main(int argc, char **argv)
{
        ExifData *ed;
        ExifEntry *entry;
        ExifByteOrder bo;
	
        if (argc < 2) {
	        printf("Usage: %s image.jpg\n", argv[0]);
	        printf("Displays tags potentially relating to ownership "
                       "of the image.\n");
	        return 1;
        }
	
        /* Load an ExifData object from an EXIF file */
        ed = exif_data_new_from_file(argv[1]);
        if (!ed) {
	        printf("File not readable or no EXIF data in file %s\n", argv[1]);
	        return 2;
        }

        bo = exif_data_get_byte_order(ed);
        switch (bo)
        {
        case EXIF_BYTE_ORDER_MOTOROLA:
                printf("Big endian\n");
                break;
        case EXIF_BYTE_ORDER_INTEL:
                printf("Small endian\n");                
                break;
        default:
                printf("Unknown byte order\n");
                break;
        }
        
        /* show_tag(ed, EXIF_IFD_0,    EXIF_TAG_ARTIST); */
        /* show_tag(ed, EXIF_IFD_0,    EXIF_TAG_COPYRIGHT); */
        /* show_tag(ed, EXIF_IFD_0,    EXIF_TAG_SOFTWARE);         */
        /* show_tag(ed, EXIF_IFD_0,    EXIF_TAG_MAKE); */
        /* show_tag(ed, EXIF_IFD_0,    EXIF_TAG_MODEL); */
        /* show_tag(ed, EXIF_IFD_0,    EXIF_TAG_ORIENTATION); */
        /* show_tag(ed, EXIF_IFD_0,    EXIF_TAG_DATE_TIME); */
        show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_SUB_SEC_TIME);
        show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_X_DIMENSION);
        show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_Y_DIMENSION);
        /* printf("\n"); */
        show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_FOCAL_LENGTH);
        show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_FNUMBER);
        show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_TIME);
         show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_ISO_SPEED_RATINGS);
        /* show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_BIAS_VALUE);  */
        /* show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_WHITE_BALANCE); */
        /* show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_PROGRAM); */
        /* show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_METERING_MODE); */
        show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_FLASH);
        /* show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_MODE); */
        /* show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_COLOR_SPACE); */
        /* show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_GAMMA); */
        /* show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_WHITE_POINT);  */
        /* Not part of JPEG EXIF, part of TIFF EXIF */
        /*show_tag(ed, EXIF_IFD_1, PIE_EXIF_TAG_LENS_1);
          show_tag(ed, EXIF_IFD_1, PIE_EXIF_TAG_LENS_2);*/

        //exif_data_dump(ed);
 	
        /* Free the EXIF data */
        exif_data_unref(ed);
 	
        return 0;
}
