#include <stdio.h>
#include <string.h>
#include "../dm/pie_exif_data.h"
#include "../exif/pie_exif.h"
#include "../cfg/pie_cfg.h"

int main(int argc, char** argv)
{
        struct pie_exif_data ped;
        int ok;

        if (argc < 1)
        {
                return 1;
        }

        if (pie_exif_load(&ped, argv[1]))
        {
                return 1;
        }

        pie_cfg_load(PIE_CFG_PATH);

        ped.ped_mob_id = 5;
        ok = pie_exif_data_create(pie_cfg_get_db(), &ped);
        if (ok)
        {
                printf("create failed %d\n", ok);
        }
        else
        {
                printf("created: %ld\n", ped.ped_mob_id);                
        }
        
        pie_exif_data_release(&ped);
        ok = pie_exif_data_read(pie_cfg_get_db(), &ped);
        if (ok)
        {
                printf("read failed %d\n", ok);
        }

        ped.ped_mob_id = 6;

        printf("%p\n", (void*)ped.ped_lens_model);
        printf("strlen: %ld\n", strlen(ped.ped_lens_model));
        
        ok = pie_exif_data_create(pie_cfg_get_db(), &ped);
        if (ok)
        {
                printf("create failed %d\n", ok);
        }
        else
        {
                printf("created: %ld\n", ped.ped_mob_id);                
        }

        
        pie_cfg_close();
        
        return 0;
}
