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
#include "pie_exif.h"

#define EXIF_TIME_FORMAT_E "YYYY:MM:DD HH:MM:SS"
#define EXIF_TIME_FORMAT "%Y:%m:%d %T"

static ExifEntry* get_entry(ExifData*, ExifIfd, ExifTag);
static uint16_t swap_uint16(uint16_t v);
static uint32_t swap_uint32(uint32_t v);
static uint16_t load_exif_uint16(const unsigned char*);
static uint32_t load_exif_uint32(const unsigned char*);

int pie_exif_load(struct pie_exif_data* ped, const char* path)
{
        char buf[256];
        ExifData* ed;
        ExifEntry* entry;
        int ret = 1;
        short sval;

        buf[255] = '\0';

        /* Clear ped first */
        memset(ped, 0, sizeof(*ped));

        ed = exif_data_new_from_file(path);
        if (ed == NULL)
        {
                return ret;
        }

        if (entry = get_entry(ed, EXIF_IFD_0, EXIF_TAG_ARTIST))
        {
                ped->ped_artist = strdup((char*)entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_0, EXIF_TAG_COPYRIGHT))
        {
                ped->ped_copyright = strdup((char*)entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_0, EXIF_TAG_SOFTWARE))
        {
                ped->ped_software = strdup((char*)entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_0, EXIF_TAG_MAKE))
        {
                ped->ped_make = strdup((char*)entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_0, EXIF_TAG_MODEL))
        {
                ped->ped_model = strdup((char*)entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_0, EXIF_TAG_ORIENTATION))
        {
                ped->ped_orientation = load_exif_uint16(entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_0, EXIF_TAG_DATE_TIME))
        {
                ped->ped_date_time = strdup((char*)entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_SUB_SEC_TIME))
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
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_X_DIMENSION))
        {
                ped->ped_x_dim = (uint32_t)load_exif_uint16(entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_Y_DIMENSION))
        {
                ped->ped_y_dim = (uint32_t)load_exif_uint16(entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_FOCAL_LENGTH))
        {
                float num = (float)load_exif_uint32(entry->data);
                float den = (float)load_exif_uint32(entry->data + 4);

                ped->ped_focal_len = (short)(num / den);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_FNUMBER))
        {
                float num = (float)load_exif_uint32(entry->data);
                float den = (float)load_exif_uint32(entry->data + 4);

                ped->ped_fnumber = (short)(num / den);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_TIME))
        {
                int num = (int)load_exif_uint32(entry->data);
                int den = (int)load_exif_uint32(entry->data + 4);

                snprintf(buf, 255, "%d/%d", num, den);
                ped->ped_exposure_time = strdup(buf);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_ISO_SPEED_RATINGS))
        {
                ped->ped_iso = (int)load_exif_uint16(entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_BIAS_VALUE))
        {
                float num = (float)load_exif_uint32(entry->data);
                float den = (float)load_exif_uint32(entry->data + 4);

                ped->ped_exposure_bias = (short)(num / den);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_WHITE_BALANCE))
        {
                ped->ped_white_balance = load_exif_uint16(entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_PROGRAM))
        {
                ped->ped_exposure_prog = load_exif_uint16(entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_METERING_MODE))
        {
                ped->ped_metering_mode = load_exif_uint16(entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_FLASH))
        {
                ped->ped_flash = load_exif_uint16(entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_MODE))
        {
                ped->ped_exposure_mode = load_exif_uint16(entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_COLOR_SPACE))
        {
                ped->ped_color_space = load_exif_uint16(entry->data);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_GAMMA))
        {
                float num = (float)load_exif_uint32(entry->data);
                float den = (float)load_exif_uint32(entry->data + 4);

                ped->ped_gamma = (int)(num / den);
        }
        if (entry = get_entry(ed, EXIF_IFD_EXIF, EXIF_TAG_WHITE_POINT))
        {
                float num = (float)load_exif_uint32(entry->data);
                float den = (float)load_exif_uint32(entry->data + 4);

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

        entry = exif_data_get_entry(ed, tag);
        /*exif_content_get_entry(d->ifd[ifd],tag);*/

        return entry;
}

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

static uint16_t load_exif_uint16(const unsigned char* d)
{
        uint16_t v;

        memcpy(&v, d, sizeof(uint16_t));

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

static uint32_t load_exif_uint32(const unsigned char* d)
{
        uint32_t v;

        memcpy(&v, d, sizeof(uint32_t));

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
