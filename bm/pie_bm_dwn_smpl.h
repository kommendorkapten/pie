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

#ifndef __PIE_DWN_SMPL_H__
#define __PIE_DWN_SMPL_H__

#include "../pie_types.h"

/**
 * Down sample an bitmap. Downscaling is performed by a Gaussian blur 
 * across the neighbours. The new size is provided by maximum new width
 * and height. The new size is the smallest of the new parameters that 
 * can be achieved by maintaining the original aspect ratio.
 * A dimension can be set to don't care by providing -1. It is an error
 * to set both directions as don't care.
 * @param output bitmap. Must be an uninitialized bitmap structure. 
 *        If initialized, memory leak will occur.
 * @param source bitmap to downsample.
 * @param maximum new width in pixels, -1 if don't care.
 * @param maximum new height in pixels, -1 if dont' care.
 * @return 0 on success. Non zero otherwise.
 */
int pie_bm_dwn_smpl(struct pie_bitmap_f32rgb* restrict,
                    const struct pie_bitmap_f32rgb* restrict,
                    int,
                    int);

#endif /* __PIE_DWN_SMPL_H__ */
