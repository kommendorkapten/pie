/*
* Copyright (C) 2016 Fredrik Skogman, skogman - at - gmail.com.
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

#ifndef PIE_DEFS_H
#define PIE_DEFS_H

#define PIE_HIST_RES 256
#define PIE_PATH_LEN 256

#define PIE_CURVE_MAX_CNTL_P 16

enum pie_channel
{
        PIE_CHANNEL_INVALID = 0,
        PIE_CHANNEL_RED = 0x1,
        PIE_CHANNEL_GREEN = 0x2,
        PIE_CHANNEL_BLUE = 0x4,
        PIE_CHANNEL_RGB = 0x7,
};


#endif /* PIE_DEFS_H */
