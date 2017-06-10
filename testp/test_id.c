#include <stdio.h>
#include <sys/time.h>
#include "../pie_id.h"
#include "../pie_types.h"

extern pie_id pie_id_from_ts(unsigned char host,
                             unsigned char worker,
                             struct timeval* tv,
                             unsigned char type);

int main(void)
{
        struct timeval tv;

        tv.tv_sec = PIE_EPOCH;
        tv.tv_usec = 0;
        printf("Id @ epoch %016lx\n",
               pie_id_from_ts(10, 1, &tv, PIE_ID_TYPE_MIN));
        tv.tv_sec++;
        tv.tv_usec = 200 * 1000;
        printf("Id @ 1.2s  %016lx\n",
               pie_id_from_ts(10, 1, &tv, PIE_ID_TYPE_MIN));
        printf("Id @ now   %016lx\n",
               pie_id_create(10, 17, PIE_ID_TYPE_MOB));

}
