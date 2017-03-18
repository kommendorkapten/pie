#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include "../lib/s_queue.h"

#define SOCKET "/tmp/pie_ingest.sock"

int main(void)
{
        ssize_t br;
        struct q_producer* q = q_new_producer(QUEUE_INTRA_HOST);
        int ok;

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
        
        int msg = 0;
        for (;;)
        {
                char buf[256];
                sprintf(buf, "%09d", msg++);
                if ((br = q->send(q->this, buf, strlen(buf) + 1)) < 0)
                {
                        perror("send");
                        return -1;
                }
                
                sleep(1);
        }

        q->close(q->this);
        q_del_producer(q);
        
        return 0;
}
