#include        <wind.h>
#include	<stdlib.h>


char *WhereIs (int now, int tot, int len)
{
  static char WherStr[101];
  int i, z = ((len = (min(100, len))) * now) / tot;
  for (i = 0; i < len; i ++) WherStr[i] = (i == z) ? 'Û' : '±';
  return WherStr;
}
