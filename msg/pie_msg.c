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
#include "pie_msg.h"

struct pie_msg* pie_msg_alloc(void)
{
        struct pie_msg* m = malloc(sizeof(struct pie_msg));

        m->buf[0] = 0;
        m->type = PIE_MSG_INVALID;
        m->img = NULL;
        m->f1 = 0.0f;
        m->f2 = 0.0f;
        m->i1 = 0;
        m->i2 = 0;

        return m;
        
}

void pie_msg_free(struct pie_msg* m)
{
        free(m);
}
