#include	<wind.h>
#include	"C:\jbw\lib\dbf\dbf_inc.h"
#include	<mem.h>


extern	int	MaxSaySize = 64;




void WindSayC (Wind *W, int x, int y, const char *s, int l)
{
  WriteWindMem(W, x, y, s, min(MaxSaySize, l));
}






Boolean WindDbfSay (Wind *W, int x, int y, int fld)
{
  switch (Work.DescPtr[fld].FldTpe)
    {
      case 'D' : WindSayC(W,x,y,DbfD2C(DbfFldPtr(fld)),strlen(DateConvr)); break;
      case 'N' :
      case 'M' :
      case 'L' :
      case 'C' :
      default  : WindSayC(W,x,y,DbfFldPtr(fld),Work.DescPtr[fld].FldLen);
		 break;
    }
  return TRUE;
}
