#ifndef __PIE_SERVER_H__
#define __PIE_SERVER_H__

struct lws_context;

struct server
{
        const char* context_root;
        struct lws_context* context;
        int port;
        volatile int run;
        int* data;
};

extern int start_server(struct server*);

#endif /* __PIE_SERVER_H__ */
