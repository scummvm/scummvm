#include	"dbf_inc.h"





Boolean CloseAll (void)
{
  WorkArea *wa = WorkPtr;
  for (WorkPtr = WorkTab; WorkPtr < WorkTab + Areas; WorkPtr ++)
    if (! DbfClose()) return FALSE;
  WorkPtr = wa;
  Err(Ok);
}
