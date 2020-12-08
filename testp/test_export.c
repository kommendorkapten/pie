#include <stdio.h>
#include "../prunt/pie_log.h"
#include "../vendor/s_queue.h"
#include "../mq_msg/pie_mq_msg.h"
#include "../encoding/pie_json.h"

int main(void)
{
        char buf[4096];
        struct q_producer* q;
        struct pie_mq_export_media msg;
        int ret = 1;
        int ok;
        size_t bw;

        snprintf(msg.path, 128, "/3.jpg");
        msg.mob_id = 85562243416131329;
        msg.stg_id = 4;
        msg.max_x = 1024;
        msg.max_y = 1024;
        msg.type = PIE_MQ_EXP_JPG;
        msg.quality = 100;
        msg.sharpen = 1;
        msg.disable_exif = 1;

        q = q_new_producer(QUEUE_INTRA_HOST);
        if (q == NULL)
        {
                PIE_ERR("Can not create queue");
                goto cleanup;
        }

        ok = q->init(q->this, Q_EXPORT_MEDIA);
        if (ok)
        {
                PIE_ERR("Can not connect to '%s'", Q_EXPORT_MEDIA);
                goto cleanup;
        }

        bw = pie_enc_json_mq_export(buf, 4096, &msg);

        q->send(q->this, buf, bw);

        ret = 0;
cleanup:
        if (q)
        {
                q->close(q->this);
                q_del_producer(q);
        }

        return ret;
}
