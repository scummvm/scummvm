#include	"dbf_inc.h"





Boolean DbfReplace (int fld, const char *s)
{
  FieldDesc *fd;
  char *p, *q;
  int fl, sl;

  // see if base opened
  UseChk;

  // get field description access
  if ((p = DbfFldPtr(fld)) == NULL) return FALSE;
  fd = &(Work.DescPtr[fld]);

  // get field length
  fl = fd->FldLen;

  // get string length (at most fl)
  q = memchr(s, '\0', fl);
  sl = (q == NULL) ? fl : q - (char *)s;

  // move those bytes
  memcpy(p, s, sl);

  // fill rest of the field with blanks
  while (sl < fl) p[sl ++] = ' ';

  // success
  return TRUE;
}
