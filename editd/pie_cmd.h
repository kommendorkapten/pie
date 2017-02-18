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

#ifndef __PIE_CMD_H__
#define __PIE_CMD_H__

#include <stddef.h>

struct pie_msg;

/**
 * Parse a command line and fill data in the pie_msg.
 * @param the pie_msg to initialize
 * @param the command string
 * @param the length of the command string, without the NULL terminator
 * @return 0 if message could be constructed from the command. Non zero
 *         otherwise.
 */
extern int parse_cmd_msg(struct pie_msg*, char*, size_t);

#endif /* __PIE_CMD_H__ */
