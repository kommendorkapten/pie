/*
* Copyright (C) 2016 Fredrik Skogman, skogman - at - gmail.com.
* This file is part of fswalk.
*
* The contents of this file are subject to the terms of the Common
* Development and Distribution License (the "License"). You may not use this
* file except in compliance with the License. You can obtain a copy of the
* License at http://opensource.org/licenses/CDDL-1.0. See the License for the
* specific language governing permissions and limitations under the License. 
* When distributing the software, include this License Header Notice in each
* file and include the License file at http://opensource.org/licenses/CDDL-1.0.
*/

#ifndef __FSWALK_H__
#define __FSWALK_H__

/**
 * Traverse the direcotry in a breadth first manner. For each file
 * call provided callback.
 * Only files and directories are supported.
 * This methor *is* entrant.
 * @param path to traverse.
 * @param callback to execute.
 * @return void
 */
void walk_dir(const char*, void (*cb)(const char*));

#endif /* __FSWALK_H__ */
