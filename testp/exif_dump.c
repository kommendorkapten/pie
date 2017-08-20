#include <stdio.h>
#include "../dm/pie_exif_data.h"
#include "../exif/pie_exif.h"
#include "../encoding/pie_json.h"

int main(int argc, char** argv)
{
        char buf[4000];
        struct pie_exif_data ed;

        if (argc < 2)
        {
	        printf("Usage: %s image\n", argv[0]);
	        return 1;
        }

        if (pie_exif_load(&ed, argv[1]))
        {
                printf("Failed to load exif data\n");
                return 1;
        }

        pie_json_enc_exif(buf, 4000, &ed);
        printf("%s\n", buf);
        
        return 0;
}
