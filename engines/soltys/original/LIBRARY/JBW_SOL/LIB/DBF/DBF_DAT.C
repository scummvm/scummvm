#include	"dbf_inc.h"





char *DbfUpdat (void)
{
  int i, yy, mm, dd;
  static char d[9];

  if (Work.Used)
    {
      yy = 1900 + Work.HeadPtr->LastUpdt[0];
      mm = Work.HeadPtr->LastUpdt[1];
      dd = Work.HeadPtr->LastUpdt[2];
      for (i = 3; i >= 0; i --)
	{
	  (d+0)[i] = '0' + yy % 10; yy /= 10;
	}
      for (i = 1; i >= 0; i --)
	{
	  (d+4)[i] = '0' + mm % 10; mm /= 10;
	  (d+6)[i] = '0' + dd % 10; dd /= 10;
	}
      d[8] = '\0';
    }
  else
    {
      strcpy(d, "00000000");
    }
  return d;
}
