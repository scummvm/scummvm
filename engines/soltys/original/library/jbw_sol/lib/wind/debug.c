#include	<wind.h>
#include	<string.h>





static	void	(*Fun)(void);




void ShowDebug (Wind *W)
{
  int i; DebugEntry *tab = W->AuxPtr;

  Fun();
  for (i = 0; i < W->Hig; i ++)
    {
      void *p = tab[i].Ptr;
      int x = W->X;

      WriteWindText(W, x, i, "              ");
      switch (tab[i].Type)
	{
	  case 1: WriteWindText(W, x+1, i, (*(char**)p == NULL) ? "NULL"
					    : (* (char **) p));
		  break;
	  case 2: WriteWindInt(W, x+5, i, * (int *) p); break;
	  case 3: WriteWindWord(W, x+6, i, * (int *) p); break;
	  case 4: WriteWindLong(W, x, i, * (long *) p); break;
	  case 5: WriteWindDword(W, x+1, i, * (long *) p); break;
	}
    }
  W->Time = TimerLimit(DEBUG_DELAY);
}






Wind * MakeDebug (DebugEntry *tab, void (*fun)(void))
{
  Wind *dw;
  DebugEntry *tp;
  int i;
  int w = 0, h = 0;

  for (tp = tab; tp->Type != 0; tp ++)
    {
      i = strlen(tp->Txt) + 12;
      if (i > w) w = i;
      ++ h;
    }
  dw = MakeWind(MaxScrWid-2-w, ZoomBot-h-1, MaxScrWid-1, ZoomBot, HLP);
  if (dw != NULL)
    {
      dw->X = w - 11;
      dw->Flags.Movable = TRUE;
      dw->ReptProc = ShowDebug;
      dw->AuxPtr = tab;
      Fun = fun;
      SetTitle(dw, "Debug", -1);
      ShowWind(dw);
      for (i = 0; i < h; i ++)
	{
	  WriteWindText(dw, 0, i, tab[i].Txt);
	}
    }
  return dw;
}
