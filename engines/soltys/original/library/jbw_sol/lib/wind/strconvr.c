#include	<wind.h>
#include	<string.h>
#include	<stdlib.h>





extern char (*Convr)(char);




char * StrConvr (char *s)
{
  char *p;
  if (Convr != NULL && s != NULL) for (p = s; *p; p ++) *p = Convr(*p);
  return s;
}
