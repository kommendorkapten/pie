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

#include <stddef.h>
#include "pie_coll_handler.h"
#include "../lib/llist.h"
#include "../encoding/pie_json.h"
#include "../dm/pie_collection.h"
#include "../dm/pie_mob.h"

#include <stdio.h>

size_t pie_coll_h_collections(char* buf, size_t len, sqlite3* db)
{
        struct llist* cl;
        struct lnode* n;
        size_t bw;

        cl = pie_collection_find_all(db);
        if (cl == NULL)
        {
                return 0;
        }
        bw = pie_json_enc_collection_list(buf, len, cl);

        n = llist_head(cl);
        while (n)
        {
                struct pie_collection* c = n->data;
                
                pie_collection_free(n->data);
                n = n->next;
        }
        llist_destroy(cl);
        
        return bw;
}

size_t pie_coll_h_collection(char* buf, size_t len, sqlite3* db, pie_id c)
{
        struct llist* ml = pie_mob_find_collection(db, c);
        struct lnode* n;
        size_t bw = 0;

        if (ml == NULL)
        {
                return 0;
        }
        bw = pie_json_enc_collection(buf, len, c, ml);

        n = llist_head(ml);
        while(n)
        {
                struct pie_mob* mob = n->data;

                pie_mob_free(mob);
                n = n->next;

        }
        llist_destroy(ml);

        return bw;
}
                                    

