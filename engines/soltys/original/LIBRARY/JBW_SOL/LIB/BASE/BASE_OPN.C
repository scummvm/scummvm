#include	<base.h>
#include	<string.h>




Wind * OpenBaseWind (WindHan *wh)
{
  int i, j, wid = 0, hig = 0;
  EdtDef *ed = wh->Fmt;
  Wind *w;

  if (wh->Wnd != NULL)
    {
      BringToFront(wh->Wnd);
      return wh->Wnd;
    }
  /// horizontal size
  for (i = 0; ed[i].Show != NULL; i ++)
    {
      j = ed[i].X + strlen(ed[i].Nam) + FormatFieldLen(wh, i);
      if (j >= wid) wid = j + 1;
      /// vertical size
      j = ed[i].Y;
      if (j >= hig) hig = j + 1;
    }
  /// create window
  w = MakeWind(0, 0, wid+1, hig+1, STD);
  if (w == NULL) NoCore();
  w->Hig = i;
  MoveWind(w, wh->X, wh->Y);
  SetTitle(w, wh->Nam, -1);

  //--- show field titles
  for (i = 0; i < w->Hig; i ++)
    {
      WriteWindText(w, ed[i].X, ed[i].Y, ed[i].Nam);
    }

  /// draw horizontal lines
  for (i = 0; i < hig; i ++)
    {
      for (j = 0; j < w->Hig; j ++) if (ed[j].Y == i) break;
      if (j == w->Hig) DrawHLine(w, i, 'Ä');
    }

  w->AuxPtr = wh;
  w->ShowProc = ShowPureChar;
  w->ReptProc = BaseRepaint;
  w->KeybProc = BaseKey;
  w->KillProc = EditShut;
  w->Flags.Movable = TRUE;
  BaseReset(w);
  return wh->Wnd = w;
}
