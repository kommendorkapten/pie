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

#ifndef __S_QUEUE_INTRA_H__
#define __S_QUEUE_INTRA_H__

#include <stddef.h>
#include "s_queue.h"

struct q_queue;

struct q_queue_intra_p
{
        char queue[MAX_QUEUE_NAME];
        char local[MAX_QUEUE_NAME];
        int fd;
};

struct q_queue_intra_c
{
        char queue[MAX_QUEUE_NAME];
        int fd;
};

/* Consumer */
/**
 * Initialise a consumer.
 * @param queue to initialise.
 * @param name of queue.
 * @return 0 if sucessfull, non-zero otherwise.
 */
extern int q_intra_c_init(struct q_queue*, char*);

/**
 * Read a record from the queue.
 * @param queue to read from.
 * @param pointer to where to store the data.
 * @param max bytes to read.
 * @return number of bytes received, negative if error occured.
 */
extern ssize_t q_intra_c_recv(struct q_queue*, char*, size_t);

/**
 * Close a queue.
 * @param queue to close.
 * @return void.
 */
extern void q_intra_c_close(struct q_queue*);

/* Producer */
/**
 * Initialise a producer.
 * @param queue to initialise.
 * @param name of queue.
 * @return 0 if sucessfull, non-zero otherwise.
 */
extern int q_intra_p_init(struct q_queue*, char*);

/**
 * Write a record from the queue.
 * @param queue to read from.
 * @param pointer to where to read data.
 * @param size of record.
 * @return number of bytes written, negative if error occured.
 */
extern ssize_t q_intra_p_send(struct q_queue*, char*, size_t);

/**
 * Close a queue.
 * @param queue to close.
 * @return void.
 */
extern void q_intra_p_close(struct q_queue*);

#endif /* __S_QUEUE_INTRA_H__ */
