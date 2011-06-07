#include	"dbf_inc.h"




void DbfStr (int fld, long n)
{
  int i = DbfFldLen(fld);
  char *s = DbfFldPtr(fld) + i - 1;
  Boolean neg = n < 0;

  if (neg) n = -n;
  while (i --)
    {
      *(s --) = '0' + n % 10;
      n /= 10;
      if (! n) break;
    }
  if (neg && i) *s = '-';
}
