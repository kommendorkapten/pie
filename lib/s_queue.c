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

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "s_queue.h"
#include "s_queue_intra.h"

struct q_consumer* q_new_consumer(enum q_type type)
{
        struct q_consumer* q = NULL;

        switch(type)
        {
        case QUEUE_INTER_HOST:
                break;
        case QUEUE_INTRA_HOST:
                q = malloc(sizeof(struct q_consumer));
                q->this = malloc(sizeof(struct q_queue_intra_c));
                q->init = &q_intra_c_init;
                q->recv = &q_intra_c_recv;
                q->close = &q_intra_c_close;
                q->fd = &q_intra_c_fd;
                memset(q->this, 0, sizeof(struct q_queue_intra_c));
                ((struct q_queue_intra_c*)q->this)->fd = -1;
                break;
        case QUEUE_AMQP:
                break;
        }

        return q;
}

void q_del_consumer(struct q_consumer* q)
{
        assert(q != NULL);
        free(q->this);
        free(q);
}

struct q_producer* q_new_producer(enum q_type type)
{
        struct q_producer* q = NULL;

        switch(type)
        {
        case QUEUE_INTER_HOST:
                break;
        case QUEUE_INTRA_HOST:
                q = malloc(sizeof(struct q_producer));
                q->this = malloc(sizeof(struct q_queue_intra_p));
                q->init = &q_intra_p_init;
                q->send = &q_intra_p_send;
                q->close = &q_intra_p_close;
                memset(q->this, 0, sizeof(struct q_queue_intra_p));
                ((struct q_queue_intra_p*)q->this)->fd = -1;
                break;
       case QUEUE_AMQP:
                break;
        }

        return q;
}

void q_del_producer(struct q_producer* q)
{
        assert(q != NULL);
        free(q->this);
        free(q);
}
