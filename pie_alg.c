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

#define  Pr  .299
#define  Pg  .587
#define  Pb  .114



//  public-domain function by Darel Rex Finley
//
//  The passed-in RGB values can be on any desired scale, such as 0 to
//  to 1, or 0 to 255.  (But use the same scale for all three!)
//
//  The "change" parameter works like this:
//    0.0 creates a black-and-white image.
//    0.5 reduces the color saturation by half.
//    1.0 causes no change.
//    2.0 doubles the color saturation.
//  Note:  A "change" value greater than 1.0 may project your RGB values
//  beyond their normal range, in which case you probably should truncate
//  them to the desired range before trying to use them in an image.

void changeSaturation(double *R, double *G, double *B, double change) {

  double  P=sqrt(
  (*R)*(*R)*Pr+
  (*G)*(*G)*Pg+
  (*B)*(*B)*Pb ) ;

  *R=P+((*R)-P)*change;
  *G=P+((*G)-P)*change;
  *B=P+((*B)-P)*change; }
