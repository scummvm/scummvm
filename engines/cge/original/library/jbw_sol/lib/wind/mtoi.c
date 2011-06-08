#include	<wind.h>

int mtoi (const char * m, int radix, int len)
{
  int i = 0;
  while (len --)
    {
      byte b = (* (m ++) | 0x20);
      b -= (b == 0x20) ? b : '0';
      if (b > 9) b -= ('a'-('9'+1));
      i = i * radix + b;
    }
  return i;
}
