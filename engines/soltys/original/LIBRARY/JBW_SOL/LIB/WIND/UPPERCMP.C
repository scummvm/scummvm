#include	<wind.h>
#include	<string.h>



int UpperCmp (char *m1, char * m2, int len)
{
  int d;
  if (len < 0) len = strlen(m1) + 1;
  while (len --)
    {
      d = Upper(* (m1 ++)) - Upper(* (m2 ++));
      if (d) break;
    }
  return d;
}
