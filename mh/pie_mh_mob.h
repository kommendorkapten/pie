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

#ifndef __PIE_MH_MOB_H__
#define __PIE_MH_MOB_H__

#include <sqlite3.h>
#include "../prunt/pie_id.h"

/**
 * Delete a single MOB. This will remove all related assets:
 * Any related database entities.
 * Any media instances including proxy and thumbnail.
 * @param handle to the database.
 * @param MOB id to purge.
 * @return 0 on success.
 */
extern int pie_mh_mob_delete(sqlite3*, pie_id);

/**
 * Move a MOB from one collection to a new.
 * @param handle to the database.
 * @param Target collection id.
 * @param Source collection id.
 * @param MOB to move.
 * @return 0 on success.
 */
extern int pie_mh_mob_move(sqlite3*, pie_id, pie_id, pie_id);

#endif /* __PIE_MH_MOB_H__ */
