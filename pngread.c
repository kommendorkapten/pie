#include <stdio.h>
#include "pie.h"

int main(int argc, char** argv)
{
        int ret;
        struct bitmap_f32rgb bm;

        if (argc != 2)
        {
                return -1;
        }

        ret = png_f32_read(&bm, argv[1]);
        bm_free_f32(&bm);
        
        return ret;
}
