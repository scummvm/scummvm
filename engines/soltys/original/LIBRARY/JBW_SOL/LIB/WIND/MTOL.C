#include	<wind.h>

long mtol (const char * m, int radix, int len)
{
  long l = 0;
  while (len --)
    {
      byte b = (* (m ++) | 0x20);
      b -= (b == 0x20) ? b : '0';
      if (b > 9) b -= ('a'-('9'+1));
      l = l * radix + b;
    }
  return l;
}
