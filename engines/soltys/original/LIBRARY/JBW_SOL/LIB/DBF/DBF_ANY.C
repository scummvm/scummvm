#include	"dbf_inc.h"



char *_DbfFldPtr (int wrk, int fld)
{
  WorkArea *wa = &WorkTab[wrk];
  char * p = NULL;

  if (wa->Used)
    {
      if (fld >= 0 && fld < wa->Fields)
	{
	  p = wa->DescPtr[fld].FldPtr;
	  if (! wa->RdOnly) p += wa->HeadPtr->RecLength;
	}
    }
  return p;
}
