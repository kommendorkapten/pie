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

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "s_queue_intra.h"

int q_intra_c_init(struct q_queue* q, char* name)
{
        struct sockaddr_un un;
        struct q_queue_intra_c* qc = (struct q_queue_intra_c*)q;
        socklen_t size;

        strncpy(qc->queue, name, MAX_QUEUE_NAME);
        qc->queue[MAX_QUEUE_NAME-1] = '\0';

        un.sun_family = AF_UNIX;
        strncpy(un.sun_path, qc->queue, MAX_QUEUE_NAME);

        qc->fd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (qc->fd  < 0)
        {
                return -1;
        }

        size = (socklen_t)(offsetof(struct sockaddr_un, sun_path) +
                           strlen(un.sun_path));
        if (bind(qc->fd, (struct sockaddr*)&un, size) < 0)
        {
                return -2;
        }

        return 0;
}

ssize_t q_intra_c_recv(struct q_queue* q, char* buf, size_t len)
{
        struct q_queue_intra_c* qc = (struct q_queue_intra_c*)q;
        ssize_t br;

        br = recv(qc->fd, buf, len, 0);

        return br;
}

void q_intra_c_close(struct q_queue* q)
{
        struct q_queue_intra_c* qc = (struct q_queue_intra_c*)q;

        if (qc->fd < 0)
        {
                return;
        }
        /* shutdown is needed on OpenBSD to terminate socket properly */
        shutdown(qc->fd, SHUT_RDWR);
        close(qc->fd);
        unlink(qc->queue);
}

int q_intra_p_init(struct q_queue* q, char* name)
{
        struct sockaddr_un un;
        struct q_queue_intra_p* qp = (struct q_queue_intra_p*)q;
        socklen_t size;

        un.sun_family = AF_UNIX;
        snprintf(qp->local, MAX_QUEUE_NAME, "%s.%05d", name, getpid());
        qp->local[MAX_QUEUE_NAME - 1] = '\0';
        strncpy(un.sun_path, qp->local, MAX_QUEUE_NAME);

        qp->fd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (qp->fd < 0)
        {
                return -1;
        }

        /* bind local socket */
        size = (socklen_t)(offsetof(struct sockaddr_un, sun_path) +
                           strlen(un.sun_path));
        if (bind(qp->fd, (struct sockaddr*)&un, size) < 0)
        {
                return -2;
        }

        /* connect to server */
        strncpy(qp->queue, name, MAX_QUEUE_NAME);
        qp->queue[MAX_QUEUE_NAME - 1] = '\0';
        memset(&un, 0, sizeof(struct sockaddr_un));
        un.sun_family = AF_UNIX;
        strncpy(un.sun_path, qp->queue, MAX_QUEUE_NAME);
        size = (socklen_t)(offsetof(struct sockaddr_un, sun_path) +
                           strlen(un.sun_path));
        if (connect(qp->fd, (struct sockaddr*)&un, size) < 0)
        {
                return -3;
        }

        return 0;
}

ssize_t q_intra_p_send(struct q_queue* q, char* buf, size_t len)
{
        struct q_queue_intra_p* qp = (struct q_queue_intra_p*)q;
        ssize_t bw;

        bw = send(qp->fd, buf, len, 0);

        return bw;
}

void q_intra_p_close(struct q_queue* q)
{
        struct q_queue_intra_p* qp = (struct q_queue_intra_p*)q;

        if (qp->fd < 0)
        {
                return;
        }
        close(qp->fd);
        unlink(qp->local);
}
