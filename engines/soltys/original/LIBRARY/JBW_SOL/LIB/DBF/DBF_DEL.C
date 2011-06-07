#include	"dbf_inc.h"






Boolean DbfDelete (Boolean on)
{
  if (Work.RdOnly) Err(FRdOnlyErr);
  if (! DbfLock()) return FALSE;
  *DbfRecPtr() = (on) ? '*' : ' ';
  return DbfUnlock();
}




Boolean DbfIsDeleted (void)
{
  if (Network) DbfSkip(0);
  return (*DbfRecPtr() == '*');
}
