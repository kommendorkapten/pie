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

#ifndef __PIE_MQ_MSG_H__
#define __PIE_MQ_MSG_H__

#include "../pie_types.h"

struct pie_mq_new_media
{
        char path[PIE_PATH_LEN];
        int storage;
};

#endif /* __PIE_MQ_MSG_H__ */
