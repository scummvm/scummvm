#include        <wind.h>
#include	<stdlib.h>
#include        <string.h>
#include	<dir.h>



char *Normalize (char *d, const char *s, const char *e)
{
  char drv[3], dir[200], nme[9], ext[5];
  int i = fnsplit(s, drv, dir, nme, ext);
  if (! (i & EXTENSION)) { *ext = '.'; strcpy(ext+1, e); }
  fnmerge(d, drv, dir, nme, ext);
  return strupr(d);
}
