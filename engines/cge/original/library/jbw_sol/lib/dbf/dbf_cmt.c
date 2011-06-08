#include	"dbf_inc.h"




Boolean DbfCommit (void)
{
  WorkArea *wa = WorkPtr;
  for (WorkPtr = WorkTab; WorkPtr < WorkTab + Areas; WorkPtr ++)
    {
      if (! WorkPtr->Used) continue;
      DbfLock();
      DbfFlush();
      DbfUnlock();
    }
  WorkPtr = wa;
  Err(Ok);
}
