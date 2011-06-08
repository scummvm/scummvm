#include	"dbf_inc.h"





Boolean DbfMatch (int fld, const char *s)
{
  FieldDesc *fd;
  char *p, *q;
  int fl, sl;

  // see if base opened
  UseChk;

  // get Work Area access
  if (! DbfLock()) return FALSE;
  DbfUnlock();

  // take field pointer
  if ((p = DbfFldPtr(fld)) == NULL) return FALSE;

  // get field description access
  fd = &(Work.DescPtr[fld]);

  // get field length
  fl = fd->FldLen;

  // get string length (at most fl)
  if ((q = memchr(s, '\0', fl+1)) == NULL) return FALSE;
  sl = q - (char *)s;

  // trailing blanks of field are ignored
  for (-- fl ; fl >= sl; fl --) if (p[fl] != ' ') return FALSE;

  // compare bytes
  return (memcmp(p, s, sl) == 0);
}
