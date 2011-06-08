#include	<wind.h>




extern	void	(*HelpProc)	(void);
extern	int	ScrollLockHelpNo = 1;
extern volatile Boolean	MouseCursorWiped;


Keys WindKeyProc (Keys k)
{
  static Boolean InHelp = FALSE;
  Wind *W = PosToWind(MX, MY);
  int wh, x = RelX(W, MX), y = RelY(W, MY);
  Boolean sl = KeyStat.ScrollLock != 0;

  _BX = (word) W;
  _CX = x+1;
  _DX = y+1;
  wh = WindHit();

  //---- disable mouse cursor?
  if (k < MouseLeft) if (! MouseCursorWiped)
    {
      MouseCursor(OFF);
      MouseCursorWiped = TRUE;
    }

  //---- mouse service for function key bar
  if (k == MouseLeft || k == TwiceLeft)
    if (KeyBarWind != NULL && W == KeyBarWind)
      if (((x+1-(x>72)) % 8) > 1)
	{
	  k = F1 + x/8;
          if (KeyStat.LShift || KeyStat.RShift) k += ShiftF1 - F1;
	  if (KeyStat.Ctrl)                     k += CtrlF1  - F1;
	  if (KeyStat.Alt)                      k += AltF1   - F1;
	  while (MousePressed(3));
	}
      else k = NoKey;

  ///---- Help
  if (k == F1 && ! InHelp && HelpProc != NULL)
    {
      int ohn = HelpNo;
      InHelp = TRUE;
      if (sl) HelpNo = ScrollLockHelpNo;
      HelpProc();
      HelpNo = ohn;
      while (MousePressed(3));
      InHelp = FALSE;
      k = NoKey;
    }

  ///---- Scroll Lock
  if (sl)
    {
      Wind *w = TopWind();
      int r, b;
      SetRefreshWind(w);
      if (w->Flags.Movable) switch (k)
	{
	  case Left  : if (w->Lft > 0) { -- w->Lft; -- w->Rgt; } k = NoKey; break;
	  case Right : if (w->Rgt < MaxScrWid-1) { ++ w->Lft; ++ w->Rgt; } k = NoKey; break;
	  case Up    : if (w->Top > ZoomTop) { -- w->Top; -- w->Bot; } k = NoKey; break;
	  case Down  : if (w->Bot < ZoomBot) { ++ w->Top; ++ w->Bot; } k = NoKey; break;
	}
      r = w->Rgt;
      b = w->Bot;
      if (w->Flags.HSizeable) switch (k)
	{
	  case Home  : if (WindBodyWid(w) > 1) -- r; break;
	  case End   : if (r < MaxScrWid-1) ++ r; break;
	}
      if (w->Flags.VSizeable) switch (k)
	{
	  case PgUp  : if (WindBodyHig(w) > 1) -- b; break;
	  case PgDn  : if (b < ZoomBot) ++ b; break;
	}
      if (r != w->Rgt || b != w->Bot)
	{
	  w->Rgt = r;
	  w->Bot = b;
	  k = NoKey;
	  w->ReptProc(w);
	}
      else SetRefreshWind(w);
    }

  switch (k)
    {
      case TwiceLeft : if (W != NULL)
			 if (wh == 1)
			   { ZoomWind(W); k = NoKey; break; }
      case MouseLeft : if (wh == 1 && W->Flags.Movable)
			 {
			   DragWind(W);
			   k = NoKey;
			   break;
			 }
		       if ((W->Flags.HSizeable && wh % 3 != 1) ||
			   (W->Flags.VSizeable && wh > 5))
			 {
			   ResizeWind(W);
			   k = NoKey;
			 }
		       break;
      case MouseRight: k = Esc; break;
    }
  return k;
}





