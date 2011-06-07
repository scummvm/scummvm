#include	<general.h>
#include	<dir.h>



char * MergeExt (char * buf, const char * nam, const char * ext)
{
  char dr[MAXDRIVE], di[MAXDIR], na[MAXFILE], ex[MAXEXT];
  fnmerge(buf, dr, di, na, (fnsplit(nam, dr, di, na, ex) & EXTENSION) ? ex : ext);
  return buf;
}