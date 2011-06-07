#include	"dbf_inc.h"





Boolean DbfString (int fld, char *s)
{
  FieldDesc *fd;
  char *p;
  int fl;
  Boolean ok = FALSE;

  // see if base opened
  UseChk;
  if (! DbfNetLock()) return FALSE;

  // get field description access
  if ((p = DbfFldPtr(fld)) == NULL) goto xit;

  fd = &(Work.DescPtr[fld]);

  // get field's trimmed length
  for (fl = fd->FldLen; fl > 0; fl --) if (p[fl-1] != ' ') break;

  // make result string
  if (fl) memcpy(s, p, fl);
  s[fl] = '\0';
  ok = TRUE;

  // return to caller
  xit:
  DbfUnlock();
  return ok;
}
