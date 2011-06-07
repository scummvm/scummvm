#include	<general.h>
#include	<dir.h>



char * ForceExt (char * buf, const char * nam, const char * ext)
{
  char dr[MAXDRIVE], di[MAXDIR], na[MAXFILE], ex[MAXEXT];
  fnsplit(nam, dr, di, na, ex);
  fnmerge(buf, dr, di, na, ext);
  return buf;
}
