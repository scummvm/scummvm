#include	<general.h>


char * dwtom (dword val, char * str, int radix, int len)
{
  while (-- len >= 0)
    {
      word w = (word) (val % radix);
      if (w > 9) w += ('A' - ('9'+1));
      str[len] = '0' + w;
      val /= radix;
    }
  return str;
}
