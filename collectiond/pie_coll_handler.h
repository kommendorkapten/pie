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

#ifndef __PIE_COLL_HANDLER_H__
#define __PIE_COLL_HANDLER_H__

#include <sqlite3.h>
#include "../pie_id.h"

/**
 * Read all collections from the provided database.
 * Encode the collections to a JSON list in the provided buffer.
 * @param buffer to hold JSON encoded collection list.
 * @param size of buffer.
 * @param database to read collections from.
 * @return number of bytes written, excluding any NULL terminator.
 */
extern size_t pie_coll_h_collections(char*, size_t, sqlite3*);

/**
 * Read a single collection from the provided database.
 * JSON encode the collection.
 * @param buffer to hold JSON encoded collection.
 * @param size of buffer.
 * @param database to read collection from.
 * @param the collection to read.
 * @return number of bytes written, excluding any NULL terminator.
 */
extern size_t pie_coll_h_collection(char*, size_t, sqlite3*, pie_id);
                                    
#endif /* __PIE_COLL_HANDLER_H__ */
