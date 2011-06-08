#include	<base.h>
#include	<string.h>



Wind * OpenBrowseWind (WindHan *wh)
{
  Wind * w;
  int i;

  if (wh->Wnd != NULL)
    {
      BringToFront(wh->Wnd);
      return wh->Wnd;
    }

  BasePush(wh->Sel);
  i = strlen(BrowseLine(wh->Fmt));
  BasePop();

  w = MakeWind(0, 0, i+1, wh->H+1, STD);
  if (w == NULL) NoCore();
  MoveWind(w, wh->X, wh->Y);
  w->Flags.Movable = TRUE;
  w->KeybProc = BrowseKey;
  w->ReptProc = BrowseRepaint;
  w->ShowProc = ShowMenuChar;
  w->KillProc = EditShut;
  w->AuxPtr = wh;
  if (wh->Mem) w->Cursor.Shape = CurIns;
  SetTitle(w, wh->Nam, -1);
  return wh->Wnd = w;
}
