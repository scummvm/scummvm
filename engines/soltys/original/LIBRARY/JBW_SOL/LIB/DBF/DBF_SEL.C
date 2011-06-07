#include	"dbf_inc.h"





Boolean DbfSelect (int w)
{
  if (w >= Areas) Err(RangeErr);
  if (w < 0) for (w = 0; w < Areas; w ++) if (! WorkTab[w].Used) break;
  if (w == Areas) Err(NoWorkErr);
  if (WorkPtr != WorkTab+w)
    {
      IPP ipp = IxPkP();
      if (ipp != NULL && ipp->Shared >= 0)
	{
	  IxFlush();
	  InitPageBoxTab(ipp->PageBoxTab);
	}
      WorkPtr = WorkTab+w;
    }
  return TRUE;
}
