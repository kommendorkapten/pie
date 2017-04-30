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

#ifndef __NEW_MEDIA_H__
#define __NEW_MEDIA_H__

struct pie_mq_new_media;

extern int pie_nm_start_workers(int);
extern int pie_nm_add_job(const struct pie_mq_new_media*);
extern void pie_nm_stop_workers(void);

#endif /* __NEW_MEDIA_H__ */
