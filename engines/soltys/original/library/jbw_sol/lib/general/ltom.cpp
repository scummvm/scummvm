#include	<general.h>


char * ltom (long val, char * str, int radix, int len)
{
  while (-- len >= 0)
    {
      int i = (int) (val % radix);
      if (i > 9) i += ('A' - ('9'+1));
      str[len] = '0' + i;
      val /= radix;
    }
  return str;
}
