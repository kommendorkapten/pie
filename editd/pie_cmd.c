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

#include <string.h>
#include <ctype.h>
#include "../pie_types.h"
#include "../pie_log.h"
#include "pie_msg.h"

#define MAX_CMD 256

/**
 * Create a channel from a single ascii character.
 * l -> PIE_CHANNEL_LUM
 * r -> PIE_CHANNEL_RED
 * g -> PIE_CHANNEL_GREEN
 * b -> PIE_CHANNEL_BLUE
 * @param ascii character, upper or lower case..
 * @return the pie channel, or PIE_CHANNEL_INVALID if invalid character
 * provided.
 */
static enum pie_channel pie_atoc(int);

int parse_cmd_msg(struct pie_msg* msg, char* data, size_t len)
{
        char copy[MAX_CMD];
        char* lasts;
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

                /* LOAD pie_id w h */
                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] (LOAD) No pie_id provided",
                                 msg->token);
                        return -1;
                }
                strncpy(msg->buf, t, PIE_MSG_BUF_LEN);

                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'",
                                 msg->token,
                                 data);
                        return -1;
                }

                w = atoi(t);
                if (w == 0)
                {
                        PIE_WARN("[%s] (LOAD)Invalid width: '%s'",
                                 msg->token,
                                 data);
                        return -1;
                }

                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'",
                                 msg->token,
                                 data);
                        return -1;
                }

                h = atoi(t);
                if (h == 0)
                {
                        PIE_WARN("[%s] (LOAD)Invalid height: '%s'",
                                 msg->token,
                                 data);
                        return -1;
                }

                msg->i1 = w;
                msg->i2 = h;
                msg->type = PIE_MSG_LOAD;
                PIE_DEBUG("[%s] Load %s %d %d",
                          msg->token,
                          msg->buf,
                          msg->i1,
                          msg->i2);
        }
        else if (strcmp(t, "VIEWP") == 0)
        {
                /* VIEWP x0, y0, x1, y1, w, h
                   Coordinates are in the image's oriented space. Not in the
                   image's physical space. */
                int val[6];

                for (int i = 0; i < 6; i++)
                {
                        if (t = strtok_r(NULL, " ", &lasts), t == NULL)
                        {
                                PIE_WARN("[%s] not a valid command '%s'",
                                         msg->token,
                                         data);
                                return -1;
                        }
                        char* p;
                        long v = strtol(t, &p, 10);

                        if (t != p)
                        {
                                val[i] = (int)v;
                        }
                        else
                        {
                                PIE_WARN("[%s] invalid argument found '%s'",
                                         msg->token,
                                         data);
                                return -1;
                        }
                }

                /* Only end coordinates may be negative */
                if (val[0] < 0 || val[1] < 0 || val[4] < 0 || val[5] < 0)
                {
                        PIE_WARN("[%s] invalid argument found '%s'",
                                 msg->token,
                                 data);
                        return -1;
                }

                msg->i1 = val[0];
                msg->i2 = val[1];
                msg->i3 = val[2];
                msg->i4 = val[3];
                msg->i5 = val[4];
                msg->i6 = val[5];

                msg->type = PIE_MSG_VIEWPORT;
                PIE_DEBUG("[%s] Set viewport: (%d, %d) (%d, %d) to (%d, %d)",
                          msg->token,
                          msg->i1,
                          msg->i2,
                          msg->i3,
                          msg->i4,
                          msg->i5,
                          msg->i6);
        }
        else if (strcmp(t, "COLORT") == 0)
        {
                /* COLORT {val}
                   val = [-30, 30] */
                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }

                char* p;
                long v = strtol(t, &p, 10);

                if (t != p && v >= -30 && v <= 30)
                {
                        msg->type = PIE_MSG_SET_COLOR_TEMP;
                        msg->f1 = (float)v / 100.f;
                        PIE_TRACE("[%s] Colortemp: %f",
                                  msg->token,
                                  msg->f1);
                }
                else
                {
                        PIE_WARN("[%s] Invalid color temp: '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }
        }
        else if (strcmp(t, "TINT") == 0)
        {
                /* TINT {val}
                   val = [-30, 30] */
                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }

                char* p;
                long v = strtol(t, &p, 10);

                if (t != p && v >= -30 && v <= 30)
                {
                        msg->type = PIE_MSG_SET_TINT;
                        msg->f1 = (float)v/ 100.f;
                        PIE_TRACE("[%s] Tint: %f",
                                  msg->token,
                                  msg->f1);
                }
                else
                {
                        PIE_WARN("[%s] Invalid tint: '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }
        }
        else if (strcmp(t, "EXPOS") == 0)
        {
                /* EXPOS 1.234
                   val = [-5.0, 5.0]*/
                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }

                char* p;
                long v = strtol(t, &p, 10);

                if (t != p && v >= -50 && v <= 50)
                {
                        msg->type = PIE_MSG_SET_EXSPOSURE;
                        msg->f1 = (float)v / 10.0f;
                        PIE_TRACE("[%s] Exposure: %f",
                                  msg->token,
                                  msg->f1);
                }
                else
                {
                        PIE_WARN("[%s] Invalid exposure: '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }
        }
        else if (strcmp(t, "CONTR") == 0)
        {
                /* CONTR {val}
                   val = [-100, 100] */
                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }

                char* p;
                long v = strtol(t, &p, 10);

                if (t != p && v >= -100 && v <= 100)
                {
                        msg->type = PIE_MSG_SET_CONTRAST;
                        msg->f1 = ((float)v + 100)/ 100.f;
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
        else if (strcmp(t, "HIGHL") == 0)
        {
                /* HIGHL {val}
                   val = [-100, 100] */
                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }

                char* p;
                long v = strtol(t, &p, 10);

                if (t != p && v >= -100 && v <= 100)
                {
                        msg->type = PIE_MSG_SET_HIGHL;
                        msg->f1 = (float)v / 100.f;
                        PIE_TRACE("[%s] Highlights: %f",
                                  msg->token,
                                  msg->f1);
                }
                else
                {
                        PIE_WARN("[%s] Invalid highlights: '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }
        }
        else if (strcmp(t, "SHADO") == 0)
        {
                /* SHADO {val}
                   val = [-100, 100] */
                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }

                char* p;
                long v = strtol(t, &p, 10);

                if (t != p && v >= -100 && v <= 100)
                {
                        msg->type = PIE_MSG_SET_SHADOW;
                        msg->f1 = (float)v / 100.f;
                        PIE_TRACE("[%s] Shadow: %f",
                                  msg->token,
                                  msg->f1);
                }
                else
                {
                        PIE_WARN("[%s] Invalid shadow: '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }
        }
        else if (strcmp(t, "WHITE") == 0)
        {
                /* WHITE {val}
                   val = [-100, 100] */
                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }

                char* p;
                long v = strtol(t, &p, 10);

                if (t != p && v >= -100 && v <= 100)
                {
                        msg->type = PIE_MSG_SET_WHITE;
                        msg->f1 = (float)v / 100.f;
                        PIE_TRACE("[%s] White: %f",
                                  msg->token,
                                  msg->f1);
                }
                else
                {
                        PIE_WARN("[%s] Invalid white: '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }
        }
        else if (strcmp(t, "BLACK") == 0)
        {
                /* BLACK {val}
                   val = [-100, 100] */
                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }

                char* p;
                long v = strtol(t, &p, 10);

                if (t != p && v >= -100 && v <= 100)
                {
                        msg->type = PIE_MSG_SET_BLACK;
                        msg->f1 = (float)v / 100.f;
                        PIE_DEBUG("[%s] Black: %f",
                                  msg->token,
                                  msg->f1);
                }
                else
                {
                        PIE_WARN("[%s] Invalid black: '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }
        }
        else if (strcmp(t, "CLARI") == 0)
        {
                /* CLARI {val}
                   val = [-100, 100] */
                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }

                char* p;
                long v = strtol(t, &p, 10);

                if (t != p && v >= -100 && v <= 100)
                {
                        msg->type = PIE_MSG_SET_CLARITY;
                        msg->f1 = (float)v / 100.f;
                        PIE_TRACE("[%s] Clarity: %f",
                                  msg->token,
                                  msg->f1);
                }
                else
                {
                        PIE_WARN("[%s] Invalid clarity: '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }
        }
        else if (strcmp(t, "VIBRA") == 0)
        {
                /* VIBRA {val}
                   val = [-100, 100] */
                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }

                char* p;
                long v = strtol(t, &p, 10);

                if (t != p && v >= -100 && v <= 100)
                {
                        msg->type = PIE_MSG_SET_VIBRANCE;
                        msg->f1 = (float)v / 100.f;
                        PIE_TRACE("[%s] Vibrance: %f",
                                  msg->token,
                                  msg->f1);
                }
                else
                {
                        PIE_WARN("[%s] Invalid vibrance: '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }
        }
        else if (strcmp(t, "SATUR") == 0)
        {
                /* SATUR {val}
                   val = [-100, 100] */
                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }

                char* p;
                long v = strtol(t, &p, 10);

                if (t != p && v >= -100 && v <= 100)
                {
                        msg->type = PIE_MSG_SET_SATURATION;
                        msg->f1 = (float)(v + 100)/ 100.f;
                        PIE_TRACE("[%s] Saturation: %f",
                                  msg->token,
                                  msg->f1);
                }
                else
                {
                        PIE_WARN("[%s] Invalid saturation: '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }
        }
        else if (strcmp(t, "ROTAT") == 0)
        {
                printf("parse ROTAT\n");
        }
        else if (strcmp(t, "CROP") == 0)
        {
                printf("parse CROP\n");
        }
        else if (strcmp(t, "SHARP") == 0)
        {
                /* SHARP amount radius threshold
                   amount    = [0, 300]
                   radius    = [0, 100]
                   threshold = [0, 20] */
                char* p;
                long v;
                float amount;
                float radius;
                float threshold;

                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'",
                                 msg->token,
                                 data);
                        return -1;
                }

                v = strtol(t, &p, 10);
                if (t != p && v >= 0 && v <= 300)
                {
                        amount = (float)v / 100.0f;
                }
                else
                {
                        PIE_WARN("[%s] Invalid amount: '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }

                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'",
                                 msg->token,
                                 data);
                        return -1;
                }
                v = strtol(t, &p, 10);
                if (t != p && v >= 1 && v <= 100)
                {
                        radius = (float)v / 10.0f;
                }
                else
                {
                        PIE_WARN("[%s] Invalid radius: '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }

                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'",
                                 msg->token,
                                 data);
                        return -1;
                }
                v = strtol(t, &p, 10);
                if (t != p && v >= 0 && v <= 20)
                {
                        threshold = (float)v;
                }
                else
                {
                        PIE_WARN("[%s] Invalid threshold: '%s'\n",
                                 msg->token,
                                 data);
                        return -1;
                }

                msg->f1 = amount;
                msg->f2 = radius;
                msg->f3 = threshold;
                msg->type = PIE_MSG_SET_SHARP;
                PIE_DEBUG("[%s] Sharp %f %f %f",
                          msg->token,
                          msg->f1,
                          msg->f2,
                          msg->f3);
        }
        else if (strcmp(t, "CURVE") == 0)
        {
                /* CURVE [l,r,g,b] x,y;(x,y);x,y */
                enum pie_channel chan;

                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'",
                                 msg->token,
                                 data);
                        return -1;
                }

                chan = pie_atoc(t[0]);
                if (chan == PIE_CHANNEL_INVALID)
                {
                        PIE_WARN("[%s] invalid channel '%s'",
                                 msg->token,
                                 t);
                        return -1;
                }

                t = strtok_r(NULL, " ", &lasts);
                if (t == NULL)
                {
                        PIE_WARN("[%s] not a valid command '%s'",
                                 msg->token,
                                 data);
                        return -1;
                }

                strncpy(msg->buf, t, PIE_MSG_BUF_LEN);
                msg->type = PIE_MSG_SET_CURVE;
                msg->i1 = chan;
        }
        else
        {
                PIE_WARN("[%s] unknown command '%s'\n",
                         msg->token,
                         data);
                return -1;
        }

        return 0;
}

static enum pie_channel pie_atoc(int c)
{
        enum pie_channel chan;

        switch (tolower(c))
        {
        case 'l':
                chan = PIE_CHANNEL_LUM;
                break;
        case 'r':
                chan = PIE_CHANNEL_RED;
                break;
        case 'g':
                chan = PIE_CHANNEL_GREEN;
                break;
        case 'b':
                chan = PIE_CHANNEL_BLUE;
                break;
        default:
                chan = PIE_CHANNEL_INVALID;
                break;
        }

        return chan;
}
