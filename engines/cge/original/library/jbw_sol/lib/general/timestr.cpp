#include	<general.h>
#include	<dos.h>





char * DateTimeString (void)
{
  static char s[] = "00-00-00 00:00:00";
  struct time t; struct date d;

  getdate(&d); gettime(&t);

  wtom(d.da_year, s + 0, 10, 2);
  wtom(d.da_mon,  s + 3, 10, 2);
  wtom(d.da_day,  s + 6, 10, 2);

  wtom(t.ti_hour, s + 9, 10, 2);
  wtom(t.ti_min,  s +12, 10, 2);
  wtom(t.ti_sec,  s +15, 10, 2);

  if (s[9] == '0') s[9] = ' ';

  return s;
}
