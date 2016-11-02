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

#include "../pie_log.h"
#include "../msg/pie_msg.h"
#include <string.h>

#define MAX_CMD 256

int parse_cmd_msg(struct pie_msg* msg, char* data, size_t len)
{
        char buf[MAX_CMD];
        char copy[MAX_CMD];
        char* lasts = buf;
        char* t;
        
        if (len >= MAX_CMD)
        {
                /* To long command */
                PIE_WARN("[%s] to long command: '%s'", 
                         msg->token,
                         data);
                return -1;
        }

        /* Len does not include NULL */
        strncpy(copy, data, len);
        copy[len] = '\0';
        t = strtok_r(copy, " ", &lasts);

        if (t == NULL)
        {
                return -1;
        }

        if (strcmp(t, "LOAD") == 0)
        {
                int w;
                int h;

                /* LOAD path w h */
                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] (LOAD) No path provided",
                                 msg->token);
                        return -1;
                }
                strncpy(msg->buf, t, PIE_MSG_BUF_LEN);
                t = strtok_r(NULL, " ", &lasts);
                if (t)
                {
                        w = atoi(t);
                        
                        if (w == 0)
                        {
                                PIE_WARN("[%s] (LOAD)Invalid width: '%s'",
                                         msg->token,
                                         data);
                                return -1;
                        }

                }
                else
                {
                        PIE_WARN("[%s] not a valid command '%s'",
                                 msg->token, 
                                 data);
                        return -1;
                }
                t = strtok_r(NULL, " ", &lasts);                
                if (t)
                {
                        h = atoi(t);
                        
                        if (h == 0)
                        {
                                PIE_WARN("[%s] (LOAD)Invalid height: '%s'",
                                         msg->token,
                                         data);
                                return -1;
                        }
                }
                else
                {
                        PIE_WARN("[%s] not a valid command '%s'",
                                 msg->token, 
                                 data);
                        return -1;
                }                

                msg->i1 = w;
                msg->i2 = h;
                msg->type = PIE_MSG_LOAD;
                PIE_TRACE("[%s] Load %s %d %d", 
                          msg->token,
                          msg->buf,
                          msg->i1,
                          msg->i2);
        }
        else if (strcmp(t, "CONTR") == 0)
        {
                /* CONTR {val} 
                   val = [-100, 100] */
                t = strtok_r(NULL, " ", &lasts);
                if (t)
                {
                        char* p;
                        long v = strtol(t, &p, 10);
                        
                        if (t != p && v >= -100 && v <= 100)
                        {
                                msg->type = PIE_MSG_SET_CONTRAST;
                                msg->f1 = (v + 100)/ 100.f;
                                PIE_TRACE("[%s] Contrast: %f", 
                                          msg->token,
                                          msg->f1);
                        }
                        else
                        {
                                PIE_WARN("[%s] Invalid contrast: '%s'\n",
                                         msg->token,
                                         data);
                                return -1;
                        }
                }
                else
                {
                        PIE_WARN("[%s] not a valid command '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }
        }
        else if (strcmp(t, "EXPOS") == 0)
        {
                printf("parse EXPOS\n");                
        }
        else if (strcmp(t, "HIGHL") == 0)
        {
                printf("parse HIGHL\n");                
        }
        else if (strcmp(t, "SHADO") == 0)
        {
                printf("parse SHADO\n");                
        }
        else if (strcmp(t, "WHITE") == 0)
        {
                printf("parse WHITE\n");                
        }
        else if (strcmp(t, "BLACK") == 0)
        {
                printf("parse BLACK\n");                
        }
        else if (strcmp(t, "CLARI") == 0)
        {
                printf("parse CLARI\n");                
        }
        else if (strcmp(t, "VIBRA") == 0)
        {
                printf("parse VIBRA\n");                
        }
        else if (strcmp(t, "SATUR") == 0)
        {
                printf("parse SATUR\n");                
        }
        else if (strcmp(t, "ROTAT") == 0)
        {
                printf("parse ROTAT\n");                
        }
        else if (strcmp(t, "CROP") == 0)
        {
                printf("parse CROP\n");                
        }
        else 
        {
                PIE_WARN("[%s] unknown command '%s'\n",
                         msg->token,
                         data);
                return -1;
        }
                
#if 0
        while (t)
        {
                printf("Got token: %s\n", t);
                t = strtok_r(NULL, " ", &lasts);
        }
#endif
        return 0;
}
