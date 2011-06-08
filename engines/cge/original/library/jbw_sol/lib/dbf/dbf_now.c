#include	"dbf_inc.h"





char *DbfToday (char *d)
{
  int i, yy, mm, dd;
  struct date da;

  getdate(&da);
  yy = da.da_year;
  mm = da.da_mon;
  dd = da.da_day;
  for (i = 3; i >= 0; i --)
    {
      (d+0)[i] = '0' + yy % 10; yy /= 10;
    }
  for (i = 1; i >= 0; i --)
    {
      (d+4)[i] = '0' + mm % 10; mm /= 10;
      (d+6)[i] = '0' + dd % 10; dd /= 10;
    }
  return d;
}
