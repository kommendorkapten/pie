float contrast_brightness(float in,
                          float c, /* [0:+] */
                          float b) /* [-1.0:1.0] */
{
        /* 
           c [0, 1] less contrast
           c [1,+] more contrast
           b is pure brightness
        */
        return c(in - 0.5f) + 0.5f + b;
}
