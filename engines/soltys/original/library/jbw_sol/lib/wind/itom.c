#include	<wind.h>



char * itom (int val, char * str, int radix, int len)
{
  while (-- len >= 0)
    {
      int i = val % radix;
      if (i > 9) i += ('A' - ('9'+1));
      str[len] = '0' + i;
      val /= radix;
    }
  return str;
}
