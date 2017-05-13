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

#ifndef __STRUTIL_H__
#define __STRUTIL_H__

/**
 * Strip away any leading white spaces.
 * @param null terminated string.
 * @return void.
 */
void strlstrip(char*);

/**
 * Strip away any ending white spaces.
 * @param null terminated string.
 * @return void.
 */
void strrstrip(char*);

/**
 * Return the file extension (the part after .) of a string.
 * The returned data is a pointer into the original string.
 * If the filename ends with a . the empty string is returned.
 * @param string
 * @return a pointer to the extension or NULL if no extension was found.
 */
const char* get_extension(const char*);

#endif /* __STRUTIL_H__ */
