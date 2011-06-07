#include	"dbf_inc.h"





char * DbfRecPtr (void)
{
  register char * p = NULL;
  if (Work.Used)
    {
      p = Work.RecPtr;
      if (! Work.RdOnly) p += Work.HeadPtr->RecLength;
    }
  return p;
}
