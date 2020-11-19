/*
* Copyright (C) 2017 Fredrik Skogman, skogman - at - gmail.com.
*
* The contents of this file are subject to the terms of the Common
* Development and Distribution License (the "License"). You may not use this
* file except in compliance with the License. You can obtain a copy of the
* License at http://opensource.org/licenses/CDDL-1.0. See the License for the
* specific language governing permissions and limitations under the License. 
* When distributing the software, include this License Header Notice in each
* file and include the License file at http://opensource.org/licenses/CDDL-1.0.
*/

#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include "strutil.h"

void strlstrip(char* s)
{
        char* p = s;
        size_t len = strlen(s);

        if (s == NULL)
        {
                return;
        }

        /* Find first occurence of a non space char */
        while (*p && isspace((int)*p))
        {
                p++;
        }

        if (p != s)
        {
                /* Move the string (including NULL term) to the beginning */
                memmove(s, p, len - (p - s) + 1);
        }
}

void strrstrip(char* s)
{
        char* last = s + strlen(s);

        while (last - 1 > s)
        {
                if (!isspace((int)*(last - 1)))
                {
                        break;
                }
                last--;
        }
        *last = '\0';
}

const char* get_extension(const char* p)
{
        const char* r = strrchr(p, '.');

        return r;
}
