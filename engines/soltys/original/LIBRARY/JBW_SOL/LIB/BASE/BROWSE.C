#include	<base.h>
#include	<string.h>
#include	<dos.h>		// only for delay()


static	char	Buff[256];
extern	char	BaseBuff[256];





char * CharStr (int chr)
{
  static char s[2] = { 0, 0 };
  *s = chr;
  return s;
}






char * FieldStr (int fld)
{
  char * p = DbfFldPtr(fld);
  int n = DbfFldLen(fld);

  if (DbfFldTpe(fld) == 'D') return DbfD2C(p);
  memcpy(BaseBuff, p, n);
  while (strlen(BaseBuff) < n) strcat(BaseBuff, " ");
  BaseBuff[n] = '\0';
  return BaseBuff;
}




char * DictStr (int cpl)
{
  int B = cpl >> 8;
  int fld = cpl & 0xFF;
  int ord;
  int n = DbfFldLen(fld);

  memcpy(BaseBuff, DbfFldPtr(fld), n);
  BaseBuff[n] = '\0';
  BasePush(B);
  ord = DbfOrder();
  SetOrder(0);		// reference number
  n = DbfFldLen(1);
  if (DbfFind(BaseBuff)) memcpy(BaseBuff, DbfFldPtr(1), n);
  else memset(BaseBuff, ' ', n);
  BaseBuff[n] = '\0';
  SetOrder(ord);
  BasePop();

  return BaseBuff;
}




char *BrowseLine (BrwDef *tab)
{
  static char s[256];
  for (s[0] = '\0'; tab->Proc != NULL; tab ++)
    strcat(s, tab->Proc(tab->Field));
  if (DbfEof()) memset(s, ' ', strlen(s));
  return s;
}






void BrowseRepaint (Wind * W)
{
  WindHan * wh = W->AuxPtr;
  int B = wh->Sel;
  int i, hig = WindBodyHig(W);
  RecPos rn;

  BasePush(B);
  DbfLock();
  GoCurrent(B);
  rn = DbfRecNo();
  if (W->Y >= hig) W->Y = hig-1;

  //---- display lines above current
  for (i = W->Y; i > 0; i --)
    {
      if (DbfBof()) break;
      DbfSkip(-1);
      WriteWindText(W, 0, i-1, BrowseLine(wh->Fmt));
    }

  //---- move displayed block up if Bof() reached
  if (i > 0 && W->Y > i)
    {
      int wid = W->Wid;
      char * p = Image(W);
      memmove(p, p + i * wid, (W->Y - i) * wid);
    }
  W->Y -= i;

  //---- display lines from current to bottom
  DbfGoto(rn);
  for (i = W->Y; i < hig; i ++)
    {
      WriteWindText(W, 0, i, BrowseLine(wh->Fmt));
      DbfSkip(1);
    }
  DbfGoto(rn);
  DbfUnlock();
  BasePop();
  W->Vp = W->Y;
  CursorGoto(W, W->Hp + W->X, W->Vp);
  SetRefreshWind(W);
}









Boolean BrowseUp (Wind *W)
{
  WindHan *wh = W->AuxPtr;
  int y = W->Y;
  int hig = WindBodyHig(W);
  int wid = WindBodyWid(W);
  Boolean ok = FALSE;

  DbfLock();
  GoCurrent(wh->Sel);
  if (DbfBof()) goto bu_xit;

  DbfSkip(-1);
  BaseSkip(wh->Sel);
  W->Flags.Repaint = FALSE;

  if (y > 0)
    {
      int l = AbsX(W, 1), t = AbsY(W, y);
      SetRefresh(l, t-1, l+wid-1, t);
      -- W->Y;
    }
  else
    {
      char * p = W->Body.Near;
      memmove(p+wid, p, (hig-1) * wid);
      SetRefreshWind(W);
    }
  ok = TRUE;

  bu_xit:
  WriteWindText(W, 0, W->Y, BrowseLine(wh->Fmt));
  DbfUnlock();
  W->X = 0;
  return ok;
}








Boolean BrowseDown (Wind *W)
{
  WindHan *wh = W->AuxPtr;
  int y = W->Y;
  int hig = WindBodyHig(W);
  int wid = WindBodyWid(W);
  Boolean ok = FALSE;

  DbfLock();
  GoCurrent(wh->Sel);
  DbfSkip(1);
  if (DbfEof())
    {
      GoCurrent(wh->Sel);;
      goto bd_xit;
    }
  BaseSkip(wh->Sel);
  W->Flags.Repaint = FALSE;

  if (y < hig - 1)
    {
      int l = AbsX(W, 1), t = AbsY(W, y);
      SetRefresh(l, t, l+wid-1, t+1);
      ++ W->Y;
    }
  else
    {
      char * p = W->Body.Near;
      memmove(p, p+wid, (hig-1) * wid);
      SetRefreshWind(W);
    }
  ok = TRUE;

  bd_xit:
  WriteWindText(W, 0, W->Y, BrowseLine(wh->Fmt));
  DbfUnlock();
  W->X = 0;
  return ok;
}








Boolean BrowsePgUp (Wind *W)
{
  WindHan *wh = W->AuxPtr;
  int hig = WindBodyHig(W);
  int i;

  DbfLock();
  GoCurrent(wh->Sel);
  for (i = hig; i > 0; i --)
    {
      if (DbfBof()) break;
      DbfSkip(-1);
    }
  BaseSkip(wh->Sel);
  DbfUnlock();
  W->X = 0;
  return TRUE;
}







Boolean BrowsePgDn (Wind *W)
{
  WindHan *wh = W->AuxPtr;
  int hig = WindBodyHig(W);
  int i;

  DbfLock();
  GoCurrent(wh->Sel);
  for (i = hig; i > 0; i --)
    {
      RecPos rn = DbfRecNo();
      DbfSkip(1);
      if (DbfEof())
	{
	  DbfGoto(rn);
	  //$$$$$$$$$$$$$$$$$$$$ W->Y = hig-1;
	  break;
	}
    }
  BaseSkip(wh->Sel);
  DbfUnlock();
  W->X = 0;
  return TRUE;
}







Boolean BrowseHome (Wind *W)
{
  WindHan *wh = W->AuxPtr;

  DbfGoTop();
  BaseSkip(wh->Sel);
  W->X = 0;
  W->Y = 0;
  return TRUE;
}





Boolean BrowseEnd (Wind *W)
{
  WindHan *wh = W->AuxPtr;

  DbfGoBottom();
  DbfSkip(-1);
  BaseSkip(wh->Sel);
  W->X = 0;
  W->Y = WindBodyHig(W)-1;
  return TRUE;
}





Boolean BrowseMouse (Wind *W)
{
  int y = -1;
  int hig = WindBodyHig(W);

  MX = MouseX(); MY = MouseY();
  if (PosToWind(MX, MY) == W)
    {
      W->X = 0;
      while (LastKey == MouseLeft || LastKey == TwiceLeft)
	{
	  y = RelY(W, MY);
	  while (W->Y < y)
	    {
	      if (! BrowseDown(W)) break;
	      if (y >= hig) break;
	    }
	  while (W->Y > y)
	    {
	      if (! BrowseUp(W)) break;
	      if (y < 0) break;
	    }
	  CursorGoto(W, W->Hp + W->X, W->Vp = W->Y);
	  Refresh();
	  if (LastKey == TwiceLeft && y == W->Y)
	    {
	      NextKey = Enter;
	      return TRUE;
	    }
	  if (! MousePressed(1)) break;
	  Idle();
	  delay(50);
	  MX = MouseX(); MY = MouseY();
	}
    }
  return TRUE;
}





Boolean BrowseLeft (Wind *W)
{
  if (W->X == 0) return FALSE;
  Buff[-- W->X] = '\0';
  return TRUE;
}





Boolean BrowseRight (Wind *W)
{
  if (++ W->X <= DbfFldLen(DbfOrder())) return TRUE;
  -- W->X;
  return FALSE;
}





Boolean BrowseClear (Wind *W)
{
  W->X = 0;
  return TRUE;
}





Boolean BrowseSearch (Wind *W)
{
  WindHan *wh = W->AuxPtr;
  int f = DbfOrder();
  char *p;
  int x;

  x = W->X;
  if (! BrowseRight(W)) return FALSE;
  p = DbfFldPtr(f);
  Buff[x ++] = Upper(LastKey);
  Buff[x] = '\0';
  DbfFind(Buff);
  if (DbfEof()) DbfSkip(-1);
  W->X = x;
  for (x = 0; x < W->X; x ++) if (Upper(p[x]) != Buff[x]) break;
  Buff[W->X = x] = '\0';
  BaseSkip(wh->Sel);
  return TRUE;
}







Boolean BrowseKey (Wind *W)
{
  WindHan * wh = W->AuxPtr;
  Boolean ok = TRUE;

  BasePush(wh->Sel);
  DbfSkip(0);
//  HoldOff(); $$$$$$$$$$$$$$$$$$$$$$$$$$$$
  if (KeyService(W))
    {
      if (TopWind() != W) goto bk_xit;
    }
  else
    {
      if (wh->Mem == 0) ok = FALSE;
      else if ((LastKey & 0xFF00) == 0 && LastKey >= 32) BrowseSearch(W);
    }
  W->Vp = W->Y;
  CursorGoto(W, W->Hp + W->X, W->Vp);
  bk_xit:
  BasePop();
  return ok;
}
