#include	<base.h>


Boolean CloseWH (Wind * W)
{
  WindHan * wh;
  int sel = DbfSelected();
  Boolean IsH = (W == NULL);

  for (wh = WHan; wh->Nam != NULL; wh ++)
    {
      Wind * w = wh->Wnd;
      if (w == NULL) continue;
      if (W == NULL || w == W)
	{
	  wh->X = w->Lft;
	  wh->Y = w->Top;
	  wh->W = w->Rgt - w->Lft - 1;
	  wh->H = w->Bot - w->Top - 1;
	  //---- single window?
	  if (W != NULL)
	    {
	      //---- parent window?
	      if (wh->Tpe == GET && wh->Lnk < 0)
		{
		  WindHan * whp;
		  //---- close all slaves
		  for (whp = WHan; whp->Nam != NULL; whp ++)
		    {
		      Wind * wp = whp->Wnd;
		      if (wp != NULL && whp->Lnk == wh->Sel) CloseWH(wp);
		    }
		}
	      //---- close side bases
	      if (wh->Tpe == GET)
		{
		  EdtDef * ed;
		  for (ed = wh->Fmt; ed->Nam != NULL; ed ++)
		    {
		      int b = ed->Base;
		      if (b != wh->Sel)
			{
			  DbfSelect(b);
			  DbfClose();
			}
		    }
		}
	      if (wh->Tpe == BRW)
		{
		  BrwDef * bd;
		  for (bd = wh->Fmt; bd->Proc != NULL; bd ++)
		    {
		      int b = bd->Field >> 8;
		      if (b)
			{
			  DbfSelect(b);
			  DbfClose();
			}
		    }
		}
	    }
	  CloseWind(w);
	  wh->Wnd = NULL;
	  IsH = TRUE;
	}
    }
  if (! IsH) CloseWind(W);
  if (W == NULL) CloseAll();
  DbfSelect(sel);
  return TRUE;
}
