#include	"btfile.h"




BT_KEYPACK far * BTFILE::Next (void)
{
  int lev = 0;
  word nxt = BT_ROOT;
  while (! Error)
    {
      BT_PAGE far * pg = GetPage(lev, nxt);
      if (pg->Hea.Down != BT_NONE)
	{
	  int i = Buff[lev].Indx;
	  nxt = (i >= 0) ? pg->Inn[i].Down : pg->Hea.Down;
	  ++ lev;
	}
      else break;
    }
  while (! Error)
    {
      BT_PAGE far * pg = GetPage(lev, nxt);
      if (pg->Hea.Down != BT_NONE)
	{
	  int i = ++ Buff[lev].Indx;
	  if (i < pg->Hea.Count)
	    {
	      nxt = pg->Inn[i].Down;
	      ++ lev;
	    }
	  else break;
	}
      else
	{
	  int i = ++ Buff[lev].Indx;
	  if (i < pg->Hea.Count) return &pg->Lea[i];
	  else nxt = Buff[-- lev].PgNo;
	}
    }
  return NULL;
}

