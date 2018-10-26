/*
* Copyright (C) 2018 Fredrik Skogman, skogman - at - gmail.com.
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

#ifndef __WORKER_H__
#define __WORKER_H__

struct wrk_pool;

/**
 * Work pool callback. This method will be called once for each job added.
 * @param pointer to the job that was added.
 * @param size of the job.
 * @retur void.
 */
typedef void (*wrk_cb)(void*, size_t);

/**
 * Create a worker pool.
 * @param number of workers to create.
 * @param callback function.
 * @return worker pool.
 */
extern struct wrk_pool* wrk_start_workers(int, wrk_cb);

/**
 * Add a job to the work pool. Work will be consumed in a FIFO manner.
 * This function may block if queue is full.
 * @param work pool.
 * @param pointer to the job to add. The job data will be copied into
 *        the queue.
 * @param size of the job.
 * @return 0 if the job was added to the queue.
 */
extern int wrk_add_job(struct wrk_pool*, const void*, size_t);

/**
 * Stop all worker and delete the pool.
 * Any ongoing job will finish before this function returns.
 * @param work pool to stop.
 * @return void.x
 */
extern void wrk_stop_workers(struct wrk_pool*);

#endif /* __WORKER_H__ */
