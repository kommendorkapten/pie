#include "../wsrv/pie_server.h"
#include <stdio.h>

int main(void)
{
        struct server server;

        server.context_root = "assets";
        server.port = 8080;

        if (start_server(&server))
        {
                printf("Failed\n");
        }

        return 0;
}
