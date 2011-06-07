#include	<wind.h>
#include	<string.h>
#include	<stdlib.h>





extern char (*Convr)(char);


char * MemConvr (char *s, int l)
{
  char *p, *q;
  if (Convr != NULL && s != NULL)
    for (q = (p = s) + l; p < q; p ++) *p = Convr(*p);
  return s;
}
