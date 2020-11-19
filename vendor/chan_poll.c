#include "chan.h"
#include "chan_def.h"
#include <poll.h>
#include <errno.h>

int chan_poll_read(struct chan* c, struct chan_msg* m, int timeout)
{
	struct pollfd pfd;
	int ready;
	int result;

	pfd.fd = c->fds[READ_FD];
	pfd.events = POLLIN | POLLHUP;

        /* TODO Update timeout if poll is restarted */
        ready = poll(&pfd, 1, timeout);
        if (ready < 0)
        {
                result = -1;
        }
        else if (ready == 0)
        {
                result = EAGAIN;
        }
        else if (pfd.revents & POLLIN) 
        {
                result = chan_read_msg(c, m);
        }
        else if (pfd.revents & POLLHUP)
        {
                result = EBADF;
        }
        else
        {
                result = -1;
        }

	return result;
}

int chan_poll_select(struct chan** c, 
                     unsigned int nc, 
                     struct chan_msg* m, 
                     int timeout)
{
	struct pollfd pfd[nc];
	int ready;
	int result = EAGAIN;
	unsigned int added = nc;

	for (unsigned int i = 0; i < nc; ++i)
	{
		pfd[i].fd = c[i]->fds[READ_FD];
		pfd[i].events = POLLIN | POLLHUP;
		pfd[i].revents = 0;
	}

	for (;;)
	{
		int done = 0;
                int possible_read = 0;

                /* TODO decrement timeout */
		ready = poll(pfd, nc, timeout);
		if (ready < 0)
		{
			result = -1;
			break;
		}
		if (ready == 0)
		{
			result = EAGAIN;
			break;
		}

		for (unsigned int i = 0; i < added; ++i)
		{
                        int ret;

			if ((pfd[i].revents & POLLIN) == 0)
			{
				continue;
			}

                        ret = chan_read_msg(c[i], m);
                        
                        if (ret == 0)
                        {
                                done = 1;
                                result = 0;
                                break;
                        }
                        else if (ret == EBADF)
			{
				/* Channel is closed. Which is strange */
				continue;
			}
			else if (errno == EAGAIN)
			{
				/* Someone stole our data.
                                   Try to read from other channels,
                                   if none succeeds, start over poll loop. */
				possible_read = 1;
			}
		}

		if (done)
		{
			break;
		}
		else if (!possible_read)
		{
			result = EBADF;
			break;
		}
	}
	return result;
}
