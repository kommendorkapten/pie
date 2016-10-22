#include "chan.h"
#include "chan_def.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

/* TODO: Better channel handling on select (remove closed etc) */

void* fun_fan_in(void*);
void* fun_fan_out(void*);
void sig_noop(int);

struct chan* chan_create(void)
{
	struct chan* c = (struct chan*) malloc(sizeof(struct chan));

        c->fan_in = NULL;
	if (c == NULL)
	{
		return NULL;
	}

	if (pipe(c->fds))
	{
		perror("chan_create::pipe");
		free(c);
		return NULL;
		
	}
		
        if (fcntl(c->fds[READ_FD], F_SETFL, O_NONBLOCK) == -1)
	{
		perror("chan_create::fncntl");
		free(c);
		return NULL;	
	}

#ifdef MT_SAFE
        c->l = lk_create();
#endif

	return c;
}

void chan_close(struct chan* c)
{
	if (c == NULL)
	{
		return;
	}

        if (c->fan_in) 
        {
                /* Kill fan in thread */
                pthread_kill(*c->fan_in, SIGUSR1);
                pthread_join(*c->fan_in, NULL);

                free(c->fan_in);
                c->fan_in = NULL;
        }

	close(c->fds[WRITE_FD]);
	c->fds[WRITE_FD] = -1;
}

void chan_destroy(struct chan* c)
{
	if (c == NULL)
	{
		return;
	}

	if (c->fds[WRITE_FD] > -1)
	{
		chan_close(c);
	}
	close(c->fds[READ_FD]);

#ifdef MT_SAFE
        lk_destroy(c->l);
        c->l = NULL;
#endif

        if (c->fan_in)
        {
                free(c->fan_in);
        }

	free(c);
}

int chan_write(struct chan* c, struct chan_msg* m)
{
	ssize_t bw;

	bw = write(c->fds[WRITE_FD], m, sizeof(struct chan_msg));
	if (bw != sizeof(struct chan_msg))
	{
		return -1;
	}

	return 0;
}

struct chan* chan_fan_in(struct chan** srcs, unsigned int num)
{
        struct chan* ret = chan_create();
        struct chan** chans = malloc((num + 2) * sizeof(struct chan*));

        chans[0] = ret;
        /* Make a copy to aoid any race conditions */
        memcpy(chans + 1, srcs, num * sizeof(struct chan*));
        chans[num + 1] = NULL;

        ret->fan_in = malloc(sizeof(pthread_t));
        /* Thread must be joinable */
        if (pthread_create(ret->fan_in, NULL, &fun_fan_in, chans))
        {
                chan_destroy(ret);
                free(chans);

                ret = NULL;
        }

        return ret;
}

int chan_fan_out(struct chan** tgts, unsigned int num, struct chan* src)
{
        pthread_attr_t attr;
        struct chan** chans = malloc((num + 2) * sizeof(struct chan*));
        pthread_t thr;
        int ret = 0;

        chans[0] = src;
        memcpy(chans + 1, tgts, num * sizeof(struct chan*));
        chans[num + 1] = NULL;

        /* Thread shall not be joinable */
        if (pthread_attr_init(&attr)) {
                free(chans);
                ret = 1;
                return  ret;
        }
        /* pthread_attr_setdetachstate can only fail on EINVAL and the values
           here are quite under control, so skip error check. */
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if (pthread_create(&thr, &attr, &fun_fan_out, chans))
        {
                free(chans);
                ret = 1;
        }

        pthread_attr_destroy(&attr);

        return ret;
}

int read_msg(struct chan* c, struct chan_msg* m)
{
	int result;
	ssize_t br;

#ifdef MT_SAFE
        if (lk_lock(c->l))
	{
		perror("chan_read::lock");
		/* FIXME */
		return -1;
	}
#endif

	br = read(c->fds[READ_FD], m, sizeof(struct chan_msg));
#ifdef MT_SAFE
        lk_unlock(c->l);
#endif
	if (br == 0)
	{
		result = EBADF;
	}
	else if (br == sizeof(struct chan_msg))
	{
		result = 0;
	}
	else 
	{
		if(errno == EBADF)
		{
			result = EBADF;
		}
		else if (errno == EAGAIN)
		{
			result = EAGAIN;
		}
		else 
		{
#ifdef __GNUC__
                        /* Some strange behavior has been noticed with gcc,
                           read returns -1 with errno as 0! Only seems to
                           happened when the channel is closed. */
                        if (br == -1 && errno == 0)
                        {
                                result = EBADF;
                        }
                        else 
                        {
                                result = -1;
                        }
#else
			result = -1;			
#endif
		}
	}
	
	return result;
}

void* fun_fan_in(void* arg)
{
        struct sigaction sa;
        sigset_t sigset;
        struct chan** chans = (struct chan**)arg;
        struct chan* tgt;
        int num = 0;
        
        /* Block all signals */
        sigfillset(&sigset);
        pthread_sigmask(SIG_BLOCK, &sigset, NULL);
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGUSR1);
        pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);
        
        /* Install dummy handler so we can abort chan_select */
        sa.sa_handler = &sig_noop;
        sa.sa_flags = 0;
        sigfillset(&sa.sa_mask);
        /* Sigaction virtally fails only on invalid signals, so skip error 
           check */
        sigaction(SIGUSR1, &sa, NULL);

        /* first chanel is target */
        tgt = chans[0];
        for (int i = 1; ; i++)
        {
                if (chans[i] == NULL)
                {
                        break;
                }
                num++;
        }

        for (;;)
        {
                struct chan_msg msg;
                int ret = chan_select(chans + 1, num, &msg, -1);

                if (ret == 0)
                {
                        chan_write(tgt, &msg);
                }
                else if (errno == EINTR)
                {
                        break;
                }
        }

        free(chans);

        return NULL;
}

void* fun_fan_out(void* arg)
{
        sigset_t sigset;
        struct chan** chans = (struct chan**)arg;
        struct chan* src;
        int num = 0;
        
        /* Block all signals */
        sigfillset(&sigset);
        pthread_sigmask(SIG_BLOCK, &sigset, NULL);

        /* first chanel is source */
        src = chans[0];
        for (int i = 1; ; i++)
        {
                if (chans[i] == NULL)
                {
                        break;
                }
                num++;
        }
        for (;;)
        {
                struct chan_msg msg;
                
                if (chan_read(src, &msg, -1))
                {
                        break;
                }
                for (int i = 0; i < num; i++)
                {
                        chan_write(chans[i + 1], &msg);
                }
        }

        free(chans);

        return NULL;
}

/* Do nothing, only needed to be able to catch SIG_USR1 so we can wake
   up blocking thread in fan_in. */
void sig_noop(int signum)
{
        /* Really stupid code. But's here just to trick compiler from
           generating warning on signum not used. */
        if (signum == SIGUSR1)
        {
                return;
        }
        return;
}
