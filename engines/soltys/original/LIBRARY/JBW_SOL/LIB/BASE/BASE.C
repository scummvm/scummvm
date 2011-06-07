#include	<base.h>
#include	<string.h>
#include	<stdlib.h>





static	int	BaseStack[64], BaseStackPtr = 0;




void BasePush (int B)
{
  if (BaseStackPtr >= ArrayCount(BaseStack)) abort();
  BaseStack[BaseStackPtr ++] = DbfSelected();
  BaseSelect(B);
}






void BasePop (void)
{
  if (BaseStackPtr <= 0) abort();
  DbfSelect(BaseStack[-- BaseStackPtr]);
}




void BaseRepaint (Wind *W)
{
  WindHan *wh = W->AuxPtr;
  int B = wh->Sel;
  EdtDef *ed = wh->Fmt;
  int i;

  BasePush(B);
  DbfLock();
  GoCurrent(B);

  if (DbfRCount() == 0) CloseWH(W);
  else for (i = 0; i < W->Hig; i ++) ed[i].Show(W, i);
  DbfUnlock();
  BasePop();
}








#pragma argsused
Boolean BasePrev (Wind * W)
{
  DbfSkip(-1);
  BaseSkip(DbfSelected());
  return TRUE;
}






#pragma argsused
Boolean BaseNext (Wind * W)
{
  DbfSkip(1);
  if (DbfEof()) DbfSkip(-1);
  BaseSkip(DbfSelected());
  return TRUE;
}







Boolean BaseHome (Wind *W)
{
  WindHan *wh = W->AuxPtr;

  DbfGoTop();
  BaseSkip(wh->Sel);
  return TRUE;
}





Boolean BaseEnd (Wind * W)
{
  WindHan *wh = W->AuxPtr;

  DbfGoBottom();
  DbfSkip(-1);
  BaseSkip(wh->Sel);
  return TRUE;
}









Boolean BaseMouse (Wind *W)
{
  WindHan *wh = W->AuxPtr;
  Wind * wx = PosToWind(MX, MY);

  if (wx == W)
    {
      int i, x = RelX(W, MX), y = RelY(W, MY);
      EdtDef *fd = wh->Fmt;
      for (i = 0; i < W->Hig; i ++)
	{
	  int x0 = fd[i].X;
	  if (y == fd[i].Y   &&
	      x >= x0        &&
	      x <  x0 + strlen(fd[i].Nam) + FormatFieldLen(wh, i))
	    {
	      BaseReset(W);
	      W->Vp = (W->Y = i);
	    }
	}
    }
  return TRUE;
}





Boolean BaseKey (Wind * W)
{
  WindHan *wh = W->AuxPtr;
  int B = wh->Sel;
//  BaseHan *bh = &Base[B];
  int i;
  EdtDef * ed = wh->Fmt;

  BasePush(B);
  //if (bh->Orders > 1) SetOrder(1); // secondary = alphabetical

  if (KeyService(W))
    {
      if (TopWind() != W) goto bk_xit;
      if (! W->Flags.EditEnable) goto bk_xit;
      BaseReset(W);
      while (ed[W->Vp].Keyb == NULL) W->Vp = ++ W->Y;
    }
  else
    if (W->Flags.EditEnable) if (ed[W->Vp].Keyb(W)) W->Flags.Touched = TRUE;

  for (i = 0; i < W->Hig; i ++) ed[i].Show(W, i);
  bk_xit:
  BasePop();
  return TRUE;
}
