#include	"dbf_inc.h"




static Boolean IxZap (void)
{
  int i, n;

  if (Work.RdOnly) Err(IxRdOnlyErr);
  n = DbfOrder();
  for (i = 0; i < MaxIxFiles; i ++)
    {
      IPP ipp;
      SetOrder(i);
      if ((ipp = IxPkP()) != NULL)
	{
	  IxFileDesc * idp = &ipp->IDesc;
	  idp->Root	= NoRec;
	  idp->Free	= NoRec;
	  idp->New	= 0;
	  ipp->IUpdt	= TRUE;
	  InitPageBoxTab(ipp->PageBoxTab);
	}
    }
  SetOrder(n);
  Err(Ok);
}








Boolean DbfZap (void)
{
  UseChk; if (Work.RdOnly) Err(FRdOnlyErr);
  if (! DbfLock()) return FALSE;
  Work.RecNo = 0; Work.HeadPtr->RecCount = 0;
  Work.AnyUpdat = TRUE;
  if (! IxZap()) return FALSE;
  DbfFlush();
  return DbfUnlock();
}
