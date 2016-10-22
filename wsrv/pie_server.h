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

#ifndef __PIE_SERVER_H__
#define __PIE_SERVER_H__

#include <signal.h>

struct chan;
struct lws_context;

struct pie_server
{
        const char* context_root;
        struct lws_context* context;
        /* server initiates commands */
        struct chan* command;
        struct chan* response;
        int port;
        volatile int run;
};

extern int start_server(struct pie_server*);

#endif /* __PIE_SERVER_H__ */
