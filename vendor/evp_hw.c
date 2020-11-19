/*
* Copyright (C) 2017 Fredrik Skogman, skogman - at - gmail.com.
*
* The contents of this file are subject to the terms of the Common
* Development and Distribution License (the "License"). You may not use this
* file except in compliance with the License. You can obtain a copy of the
* License at http://opensource.org/licenses/CDDL-1.0. See the License for the
* specific language governing permissions and limitations under the License.
* When distributing the software, include this License Header Notice in each
* file and include the License file at http://opensource.org/licenses/CDDL-1.0.
*/

#include <stdio.h>
#include <openssl/engine.h>

int evp_enable_hw(int enable)
{
        int status = 1;

        (void)enable;
#if __sun && __sparc
        int ok = 0;

        if (enable)
        {
                ENGINE* dyn;

                ENGINE_load_openssl();
                ENGINE_load_dynamic();
                ENGINE_load_cryptodev();
                ENGINE_load_builtin_engines();
                OpenSSL_add_all_algorithms();

                printf("Enable hardware SSL EVP crypto provider\n");
                dyn = ENGINE_by_id("dynamic");
                if (dyn == NULL)
                {
                        printf("No dynamic engine support found\n");
                }
                else
                {
                        ok = ENGINE_ctrl_cmd_string(dyn,
                                                    "SO_PATH",
                                                    "/lib/openssl/engines/64/libpk11.so",
                                                    0);
                        if (!ok) goto evp_done;
                        ok = ENGINE_ctrl_cmd_string(dyn,
                                                    "LOAD",
                                                    NULL,
                                                    0);
                        if (!ok) goto evp_done;
                        ok = ENGINE_init(dyn);
                        if (!ok) goto evp_done;
                        ok = ENGINE_set_default(dyn, ENGINE_METHOD_ALL & ~ENGINE_METHOD_RAND);

                evp_done:
                        if (!ok)
                        {
                                printf("Failed to initialize EVP HW provider\n");
                        }
                        else
                        {
                                status = 0;
                        }
                }
        }
#endif

        return status;
}
