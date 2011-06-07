#include	<wind.h>
#include	<dos.h>
#include	<string.h>




static char DT[] = " 00-00-00 00:00 ";

static void itom_2 (int n, char * s)
{
  itom(n, s, 10, 2);
}


static void ShowClock (Wind *w)
{
  static byte sec = (byte) -1;
  struct time t; struct date d;

  gettime(&t);
  if (t.ti_sec != sec)
    {
      sec = t.ti_sec;
      itom_2(t.ti_hour, DT+10);
      itom_2(t.ti_min,  DT+13);
      if (DT[10] == '0') DT[10] = ' ';
      DT[12] ^= (':' ^ ' ');

      getdate(&d);
      itom_2(d.da_day,  DT+ 1);
      itom_2(d.da_mon,  DT+ 4);
      itom_2(d.da_year, DT+ 7);
      if (DT[1] == '0') DT[1] = ' ';

      SetRefreshWind(w);
    }
  w->Flags.Repaint = TRUE;
}



Wind * MakeClock (int l, int t, ColorBank c, Boolean day)
{
  register char *p = (day) ? DT : (DT + 10);
  Wind *w = CreateWind(l, t, l+strlen(p)-1, t, c, FALSE, FALSE);
  if (w != NULL)
    {
      Image(w) = p;
      w->ReptProc = ShowClock;
      ShowWind(w);
    }
  return w;
}
