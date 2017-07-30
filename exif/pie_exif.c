/*
* Copyright (C) 2017 Fredrik Skogman, skogman - at - gmail.com.
* This file is part of pie project
*
* The contents of this file are subject to the terms of the Common
* Development and Distribution License (the "License"). You may not use this
* file except in compliance with the License. You can obtain a copy of the
* License at http://opensource.org/licenses/CDDL-1.0. See the License for the
* specific language governing permissions and limitations under the License.
* When distributing the software, include this License Header Notice in each
* file and include the License file at http://opensource.org/licenses/CDDL-1.0.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <libexif/exif-tag.h>
#include <libexif/exif-data.h>
#include "../dm/pie_exif_data.h"
#include "../pie_log.h"
#include "pie_exif.h"

#define EXIF_TIME_FORMAT_E "YYYY:MM:DD HH:MM:SS"
#define EXIF_TIME_FORMAT "%Y:%m:%d %T"

static ExifEntry* get_entry(ExifData*, ExifIfd, ExifTag);

static int16_t swap_int16(int16_t v);
static int32_t swap_int32(int32_t v);
/**
 * Load an integer of unknown size (max 4 bytes).
 */
static int load_exif_int(const ExifEntry*, int);
static int16_t load_exif_int16(const unsigned char*, int);
static int32_t load_exif_int32(const unsigned char*, int);

#ifdef __BYTE_ORDER__
# if __BYTE_ORDER__ == 4321
static int this_endian = EXIF_BYTE_ORDER_MOTOROLA;
# else
static int this_endian = EXIF_BYTE_ORDER_INTEL;
# endif
#else
#  error __BYTE_ORDER__ not defined
#endif

int pie_exif_load(struct pie_exif_data* ped, const char* path)
{
        char buf[256];
        ExifData* ed;
        ExifEntry* entry;
        int swap;
        int ret = 1;

        buf[255] = '\0';

        /* Clear ped first */
        memset(ped, 0, sizeof(*ped));

        ed = exif_data_new_from_file(path);
        if (ed == NULL)
        {
                return ret;
        }
        if (this_endian == exif_data_get_byte_order(ed))
        {
                swap = 0;
        }
        else
        {
                swap = 1;
        }

        if ((entry = get_entry(ed, EXIF_IFD_0, EXIF_TAG_ARTIST)))
        {
                ped->ped_artist = strdup((char*)entry->data);
        }
        if ((entry = get_entry(ed, EXIF_IFD_0, EXIF_TAG_COPYRIGHT)))
        {
                ped->ped_copyright = strdup((char*)entry->data);
        }
        if ((entry = get_entry(ed, EXIF_IFD_0, EXIF_TAG_SOFTWARE)))
        {
                ped->ped_software = strdup((char*)entry->data);
        }
        if ((entry = get_entry(ed, EXIF_IFD_0, EXIF_TAG_MAKE)))
        {
                ped->ped_make = strdup((char*)entry->data);
        }
        if ((entry = get_entry(ed, EXIF_IFD_0, EXIF_TAG_MODEL)))
        {
                ped->ped_model = strdup((char*)entry->data);
        }
        if ((entry = get_entry(ed, EXIF_IFD_0, EXIF_TAG_ORIENTATION)))
        {
                ped->ped_orientation = (short)load_exif_int(entry, swap);
        }
        if ((entry = get_entry(ed, EXIF_IFD_0, EXIF_TAG_DATE_TIME)))
        {
                ped->ped_date_time = strdup((char*)entry->data);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_SUB_SEC_TIME)))
        {
                char* c;
                char* p = (char*)entry->data;
                long v = strtol(p, &c, 10);

                /* Two digits, hundreds of a second */
                if (p != c)
                {
                        ped->ped_sub_sec_time = (int)v;
                }
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_X_DIMENSION)))
        {
                ped->ped_x_dim = load_exif_int(entry, swap);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_Y_DIMENSION)))
        {
                ped->ped_y_dim = load_exif_int(entry, swap);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_FOCAL_LENGTH)))
        {
                float num = (float)load_exif_int32(entry->data, 0);
                float den = (float)load_exif_int32(entry->data + 4, 0);

                ped->ped_focal_len = (short)(num / den);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_FNUMBER)))
        {
                float num = (float)load_exif_int32(entry->data, 0);
                float den = (float)load_exif_int32(entry->data + 4, 0);

                ped->ped_fnumber = (short)(num * 10.0f / den);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_TIME)))
        {
                int num = (int)load_exif_int32(entry->data, 0);
                int den = (int)load_exif_int32(entry->data + 4, 0);

                if (num > 100 && den > 1000) {
                        den = den / num;
                        num = 1;
                }

                snprintf(buf, 255, "%d/%d", num, den);
                ped->ped_exposure_time = strdup(buf);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_ISO_SPEED_RATINGS)))
        {
                ped->ped_iso = load_exif_int(entry, swap);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_BIAS_VALUE)))
        {
                float num = (float)load_exif_int32(entry->data, 0);
                float den = (float)load_exif_int32(entry->data + 4, 0);

                ped->ped_exposure_bias = (short)(num * 100.0f / den);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_WHITE_BALANCE)))
        {
                ped->ped_white_balance = (short)load_exif_int(entry, swap);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_PROGRAM)))
        {
                ped->ped_exposure_prog = (short)load_exif_int(entry, swap);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_METERING_MODE)))
        {
                ped->ped_metering_mode = (short)load_exif_int(entry, swap);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_FLASH)))
        {
                ped->ped_flash = (short)load_exif_int(entry, swap);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_MODE)))
        {
                ped->ped_exposure_mode = (short)load_exif_int(entry, swap);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_COLOR_SPACE)))
        {
                ped->ped_color_space = (short)load_exif_int(entry, swap);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_GAMMA)))
        {
                float num = (float)load_exif_int32(entry->data, 0);
                float den = (float)load_exif_int32(entry->data + 4, 0);

                ped->ped_gamma = (int)(num / den);
        }
        if ((entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_WHITE_POINT)))
        {
                float num = (float)load_exif_int32(entry->data, 0);
                float den = (float)load_exif_int32(entry->data + 4, 0);

                ped->ped_white_point = (int)(num / den);
        }

        exif_data_unref(ed);

        ret = 0;
        return ret;
}

time_t pie_exif_date_to_millis(const char* date, int sub_sec)
{
        struct tm tm;
        time_t ret = 0;

        if (strptime(date, EXIF_TIME_FORMAT, &tm))
        {
                /* auto detect daylight savings */
                tm.tm_isdst = -1;
                ret = mktime(&tm) * 1000 + sub_sec * 10;
        }

        return ret;
}

static ExifEntry* get_entry(ExifData* ed, ExifIfd id, ExifTag tag)
{
        ExifEntry *entry;

        (void)id;
        entry = exif_data_get_entry(ed, tag);
        /*exif_content_get_entry(d->ifd[ifd],tag);*/

        return entry;
}

static int16_t swap_int16(int16_t v)
{
        return (int16_t)((0xff00 & v) >> 8 |
                          (0x00ff & v) << 8);

}

static int32_t swap_int32(int32_t v)
{
        return (int32_t)((0xff000000 & v) >> 24 |
                         (0x00ff0000 & v) >> 8 |
                         (0x0000ff00 & v) << 8 |
                         (0x000000ff & v) << 24);
}

static int16_t load_exif_int16(const unsigned char* d, int swap)
{
        int16_t v;

        memcpy(&v, d, sizeof(v));

        if (swap)
        {
                v = swap_int16(v);
        }

        return v;
}

static int32_t load_exif_int32(const unsigned char* d, int swap)
{
        int32_t v;

        memcpy(&v, d, sizeof(v));

        if (swap)
        {
                v = swap_int32(v);
        }

        return v;
}

static int load_exif_int(const ExifEntry* entry, int swap)
{
        int ret;

        switch (entry->format)
        {
        case EXIF_FORMAT_SHORT:
                ret = (int)load_exif_int16(entry->data, swap);
                break;
        case EXIF_FORMAT_LONG:
                ret = (int)load_exif_int32(entry->data, swap);
                break;
        default:
                PIE_ERR("Unexpected EXIF format: %d", entry->format);
                abort();
        }

        return ret;
}
