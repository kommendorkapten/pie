#include <stdio.h>
#include "pie_io.h"

int main(int argc, char** argv)
{
        if (argc != 2)
        {
                return -1;
        }

        return png_f32_read(NULL, argv[1]);
}
