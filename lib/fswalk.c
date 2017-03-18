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

#include "fswalk.h"
#include "llist.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#ifdef __NON_POSIX__
#include <sys/dir.h>
#endif
#include <string.h>
#include <errno.h>

#define MAX_PATH 256

/**
 * Scan a single directory. 
 * For all directories find, add them to search list, for all files 
 * call callback with path to file.
 * @param path to scan.
 * @param list to push new directories to.
 * @param callback to execute.
 * @return 0 on success, otherwise errno from offending syscall.
 */
static int walk_single_dir(const char*, struct
			   llist*,
			   void (*cb)(const char*));

void walk_dir(const char* p, void (*cb)(const char*))
{
	struct llist* list;
	char* path;
	size_t s;
	
	list = llist_create();

	s = strlen(p) + 1;
	path = malloc(s);
	memcpy(path, p, s);
	llist_pushb(list, path);

	while (llist_size(list) > 0)
	{
		path = llist_pop(list);
		if (walk_single_dir(path, list, cb))
		{
			perror("walk_single_dir");
		}
		free(path);
	}

	llist_destroy(list);
}

static int walk_single_dir(const char* p,
			   struct llist* l,
			   void (*cb)(const char*))
{
	char path_buf[MAX_PATH];
	struct dirent* curr;
	DIR* dir;
	char* path;	
	size_t s;

	dir = opendir(p);
	if (dir == NULL)
	{
		return errno;
	}

	/* Perform a depth first search.
	 * Change llist_pushf to llist_pushb for breadth first.
	 * Depth first should be a tad faster, but typically not noticable
	 * in real a real world scenario. */
	while ((curr = readdir(dir)) != NULL)
	{
		if ((strcmp(curr->d_name, ".") == 0) ||
		    (strcmp(curr->d_name, "..") == 0))
		{
			continue;
		}

#ifdef __NON_POSIX__
		switch (curr->d_type)
		{
		case DT_DIR:
			s = strlen(p) + strlen(curr->d_name) + 2;
			path = malloc(s);
			snprintf(path,
				 s,
				 "%s/%s",
				 p, curr->d_name);
			llist_pushf(l, path);
			break;
		case DT_REG:
			snprintf(path_buf, MAX_PATH, "%s/%s", p, curr->d_name);
			cb(path_buf);
			break;
		default:
			break;
		}
#else
                struct stat stat_buf;
                snprintf(path_buf, MAX_PATH, "%s/%s", p, curr->d_name);
                stat(path_buf, &stat_buf);
                if (S_ISDIR(stat_buf.st_mode))
                {
			s = strlen(path_buf) + 1;
			path = malloc(s);
                        memcpy(path, path_buf, s);
			llist_pushf(l, path);                        
                }
                else if (S_ISREG(stat_buf.st_mode))
                {
			cb(path_buf);                        
                }
#endif
	}
	
	closedir(dir);

	return 0;
}
