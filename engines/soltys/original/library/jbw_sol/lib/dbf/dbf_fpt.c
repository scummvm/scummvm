#include	"dbf_inc.h"






char *DbfFldPtr(int n)
{
  char * p = NULL;
  if (Work.Used)
    {
      if (n >= 0 && n < Work.Fields)
	{
	  p = Work.DescPtr[n].FldPtr;
	  if (! Work.RdOnly) p += Work.HeadPtr->RecLength;
	}
    }
  return p;
}
