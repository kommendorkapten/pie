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

#ifndef __PIE_ID_H__
#define __PIE_ID_H__

#include <stdint.h>

enum pie_obj_type
{
        PIE_ID_TYPE_INVALID = 0x0,
        PIE_ID_TYPE_MOB = 0x1,
        PIE_ID_TYPE_MIN = 0x2,
        PIE_ID_TYPE_COLL = 0x3,
        PIE_ID_TYPE_PRESET = 0x4
};

typedef int64_t pie_id;

/**
 * Create a new pie_id.
 * @param host that created the object.
 * @param worker on the host that created the object.
 * @param the type of object.
 * @return a new pie id > 0 if ok, 0 invalid arguments were provided.
 */
extern pie_id pie_id_create(unsigned char, unsigned char, enum pie_obj_type);

#endif /* __PIE_ID_H__ */
