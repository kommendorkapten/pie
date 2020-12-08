#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "../vendor/s_queue.h"

#define SOCKET "/tmp/pie_ingest.sock"

/**
 * Signal handler.
 * @param the signum.
 * @return void.
 */
static void sig_h(int);

/**
 * Interrupt handler.
 * @param struct server.
 * @return NULL if successfull.
 */
static void* i_handler(void*);

int main(void)
{
        char buf[256];
        pthread_t thr_int;
        sigset_t b_sigset;
        ssize_t br;
        int ok;
        struct q_consumer* q = q_new_consumer(QUEUE_INTRA_HOST);

        if (q == NULL)
        {
                printf("Got null\n");
                return -1;
        }

        ok = q->init(q->this, SOCKET);
        if (ok)
        {
                perror("init");
                printf("%d\n", ok);
                return -1;
        }

        /* interrupt handler thread */
        ok = pthread_create(&thr_int,
                            NULL,
                            &i_handler,
                            (void*)q);
        if (ok)
        {
                printf("pthread_create:thr_int: %d\n", ok);
                return -1;
        }

        /* Block all signals during thread creating */
        sigfillset(&b_sigset);
        pthread_sigmask(SIG_BLOCK, &b_sigset, NULL);


        while ((br = q->recv(q->this, buf, 256)) > 0)
        {
                printf("%s\n", buf);
        }

        q_del_consumer(q);

        return 0;
}

static void* i_handler(void* arg)
{
        struct sigaction sa;
        void* ret = NULL;
        struct q_consumer* q = (struct q_consumer*)arg;

        /* Set up signal handler */
        sa.sa_handler = &sig_h;
        sa.sa_flags = 0;
        sigfillset(&sa.sa_mask);
        if (sigaction(SIGINT, &sa, NULL))
        {
                perror("i_handler::sigaction");
                return (void*)0x1L;
        }

        pause();
        printf("Leaving.\n");
        q->close(q->this);

        return ret;
}

static void sig_h(int signum)
{
        (void)signum;
}
