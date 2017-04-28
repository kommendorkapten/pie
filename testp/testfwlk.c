#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/evp.h>
#include <openssl/engine.h>
#include "../lib/fswalk.h"

#define NUM_PATH 256
#define MAX_PATH 256
#define BUF_LEN 8192

void cb_fun(const char*);

int main(int argc, char** argv)
{
	char path[MAX_PATH];

        if (argc > 2 )
        {
                printf("load engine\n");

                ENGINE_load_openssl();
                ENGINE_load_dynamic();
                ENGINE_load_cryptodev();
                ENGINE_load_builtin_engines();        
                OpenSSL_add_all_algorithms();

                ENGINE* dyn;
                int ok;
        
                dyn = ENGINE_by_id("dynamic");
                if (dyn == NULL)
                {
                        abort();
                }
                ok = ENGINE_ctrl_cmd_string(dyn, "SO_PATH", "/lib/openssl/engines/64/libpk11.so", 0);
                if (!ok) printf("Failed to init\n");
                ok = ENGINE_ctrl_cmd_string(dyn, "LOAD", NULL, 0);
                if (!ok) printf("Failed to init\n");
                
                if (!ENGINE_init(dyn))
                {
                        printf("Failed to init\n");
                }
                ENGINE_set_default(dyn, ENGINE_METHOD_ALL & ~ENGINE_METHOD_RAND);

        }        
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
        ssize_t size = 0;
        unsigned char sum[EVP_MAX_MD_SIZE];
        unsigned int md_len;

        EVP_MD_CTX* mdctx;
        const EVP_MD* md = EVP_sha1();
        ENGINE* impl = NULL;

        
        
        mdctx = EVP_MD_CTX_create();
        EVP_DigestInit_ex(mdctx, md, impl);
        
        while ((br = read(fd, buf, BUF_LEN)) > 0)
        {
                EVP_DigestUpdate(mdctx, buf, br);
                size += br;
        }
       
        close(fd);

        EVP_DigestFinal_ex(mdctx, sum, &md_len);
        EVP_MD_CTX_destroy(mdctx);
        
        printf("%s %ld\n", p, size);
        
        for (unsigned int i = 0; i < md_len; i++)
        {
                printf("%02x", sum[i]);
        }

        printf("\n");
}
