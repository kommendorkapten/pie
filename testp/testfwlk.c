#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include "../lib/fswalk.h"

#define NUM_PATH 256
#define MAX_PATH 256
#define BUF_LEN 4096

void cb_fun(const char*);

int main(int argc, char** argv)
{
	char path[MAX_PATH];

	if (argc > 1)
	{
		strncpy(path, argv[1], MAX_PATH);
	}
	else
	{
		strcpy(path, ".");
	}

	walk_dir(path, &cb_fun);
	
	return 0;
}

void cb_fun(const char* p)
{
#if 0
	struct stat stat_buf;

	if (stat(p, &stat_buf))
	{
		perror("stat");
		return;
	}
	printf("%s %lld\n", p, stat_buf.st_size);
#endif
        
        char buf[BUF_LEN];
        int fd = open(p, O_RDONLY);
        ssize_t br;
        int size = 0;
        unsigned char sum[20];

        
        SHA_CTX ctx;

        SHA1_Init(&ctx);

        while ((br = read(fd, buf, BUF_LEN)) > 0)
        {
                SHA1_Update(&ctx, (void*)buf, br);
                size += br;
        }
       
        close(fd);

        SHA1_Final(sum, &ctx);
        
        printf("%s %d\n", p, size);
        
        for (int i = 0; i < 20; i++)
        {
                printf("%02x", sum[i]);
        }

        printf("\n");
}
