#include	"dbf_inc.h"







Boolean _DbfBof (void)
{
  Boolean b;
  IPP ipp;
  if (! DbfLock()) return FALSE;
  ipp = IxPkP();
  b = (Work.HeadPtr->RecCount == 0L) ? TRUE :
      ((ipp == NULL) ? (Work.RecNo == 0)
		     : (IsLeaf(CurLev)				&&
		       ipp->PageBoxTab[CurLev].CurOff == 0	&&
		       ipp->PageBoxTab[CurLev].Page.LeftPage == NoRec));
  DbfUnlock();
  return b;
}
