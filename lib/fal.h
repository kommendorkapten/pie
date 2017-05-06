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

#ifndef __FAL_H__
#define __FAL_H__

#include <sys/types.h>

/**
 * Create a new directory tree in an existing one.
 * @param path to an existing directory.
 * @param new directory path to create
 * @return 0 on success, -1 otherwise.
 */
extern int fal_mkdir_tree(const char*, const char*);

/**
 * Copy data from a file descriptor to another.
 * @param the destination file descriptor.
 * @param the source file descriptor.
 * @return the number of bytes copied, or negative if an error occured.
 */
extern ssize_t fal_copy_fd(int, int);

#endif /* __FAL_H__ */
