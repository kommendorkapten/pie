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

#ifndef __PIE_BLUR_H__
#define __PIE_BLUR_H__

void pie_box_blur6(float* restrict img,
                   float* restrict buf,
                   float sigma,
                   int w,
                   int h,
                   int s);

#endif /* __PIE_BLUR_H__ */
