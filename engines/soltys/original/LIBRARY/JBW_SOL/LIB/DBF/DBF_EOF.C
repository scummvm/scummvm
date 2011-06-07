#include	"dbf_inc.h"







Boolean _DbfEof (void)
{
  Boolean e;
  if (! DbfNetLock()) return FALSE;
  e = (Work.RecNo == Work.HeadPtr->RecCount);
  DbfUnlock();
  return e;
}
