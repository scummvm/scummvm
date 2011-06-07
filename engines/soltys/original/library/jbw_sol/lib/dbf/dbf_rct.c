#include	"dbf_inc.h"




RecPos DbfRCount (void)
{
  RecPos c;
  if (! DbfNetLock()) return FALSE;
  c = (Work.Used) ? Work.HeadPtr->RecCount : (RecPos) 0;
  DbfUnlock();
  return c;
}
