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

#ifndef __S_QUEUE_H__
#define __S_QUEUE_H__

#include <sys/types.h>

#define MAX_QUEUE_NAME 64

enum q_type
{
        QUEUE_INTER_HOST = 0,
        QUEUE_INTRA_HOST,
        QUEUE_AMQP
};

struct q_queue
{
        char queue[MAX_QUEUE_NAME];
};

struct q_consumer
{
        struct q_queue* this;
        int (*init)(struct q_queue*, char*);
        ssize_t (*recv)(struct q_queue*, char*, size_t);
        void (*close)(struct q_queue*);
};

struct q_producer
{
        struct q_queue* this;
        int (*init)(struct q_queue*, char*);
        ssize_t (*send)(struct q_queue*, char*, size_t);
        void (*close)(struct q_queue*);
};

/**
 * Create a new consumer for the provided queue.
 * @param type of queue.
 * @return a consumer to start read from, NULL if queue could not be created.
 */
extern struct q_consumer* q_new_consumer(enum q_type type);

/**
 * Delete and free the memory of a consumer.
 * @param the consumer to delete.
 * @return void.
 */
extern void q_del_consumer(struct q_consumer*);

/**
 * Create a new provider for the provided queue.
 * @param type of queue.
 * @return a provider to start write to, NULL if queue could not be created.
 */
extern struct q_producer* q_new_producer(enum q_type type);

/**
 * Delete and free the memory of a producer.
 * @param the producer to delete.
 * @return void.
 */
extern void q_del_producer(struct q_producer*);

#endif /* __S_QUEUE_H__ */
