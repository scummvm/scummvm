#include	<wind.h>
#include	<string.h>
#include	<stdlib.h>






int Question (ColorBank c, const char *m, const char *q)
{
  Wind *w;
  int ml = strlen(m), ql = strlen(q),
      i = max(ml, ql) + 4;

  SetRefreshWind(TopWind());
  w = MakeWind(0, 0, i-1, 4, c);
  w->ShowProc = ShowHMChar;
  w->Flags.Movable = TRUE;
  StdMove(w);
  i = i / 2 - 1;
  WriteWindText(w, i - ml/2, 0, m);
  WriteWindText(w, i - ql/2, 2, q);
  w->Vp = 0;		//---  initial menu position
  w->Y = 2;		//---  window body row
  ShowWind(w);
  ClearKeyboard();
  i = HorzChoice(w);
  CloseWind(w);
  return i;
}
