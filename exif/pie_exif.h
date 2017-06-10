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

#ifndef __PIE_EXIF_H__
#define __PIE_EXIF_H__

#include <time.h>

enum pie_exif_orientation
{
        PIE_EXIF_ORIENTATION_CW0 = 1,
        PIE_EXIF_ORIENTATION_CW90 = 8,
        PIE_EXIF_ORIENTATION_CW180 = 3,
        PIE_EXIF_ORIENTATION_CW270 = 6,
};

struct pie_exif_data;

/**
 * Load exif data from path.
 * @param pie exif data struct to load with data.
 * @param path to file.
 * @return 0 on success.
 */
extern int pie_exif_load(struct pie_exif_data*, const char*);

/**
 * Convert exif date to epoch milliseconds.
 * @param Exif date in format: '2017:02:20 09:55:01'
 * @param Sub second timing in hundreds of a second.
 * @return number of milliseoncds since the epoch, or zero if date
 *         could not be parsed.
 */
extern time_t pie_exif_date_to_millis(const char*, int);

#endif /* __PIE_EXIF_H__ */
