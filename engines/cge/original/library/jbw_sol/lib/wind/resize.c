#include	<wind.h>


extern	void    (*IdleProc)	(void);


void ResizeWind (Wind *W)
{
  int lt, rt, bm;
  Boolean v = (W->Flags.VSizeable && MY == W->Bot),
	  l = (W->Flags.HSizeable && MX == W->Lft),
	  r = (W->Flags.HSizeable && MX == W->Rgt);
  word cursor = W->Cursor.Shape;

  W->Cursor.Shape = OFF_Cursor;

  while (MousePressed(1))
    {
      if (IdleProc != NULL) IdleProc();
      Refresh();
      MX = MouseX();
      MY = MouseY();
      lt = W->Lft;
      rt = W->Rgt;
      bm = W->Bot;
      if (l && MX < rt-1) W->Lft = MX;
      if (r && MX > lt+1) W->Rgt = MX;
      if (v && MY > W->Top+1 && MY >= ZoomTop && MY <= ZoomBot) W->Bot = MY;
      if (W->Lft != lt || W->Rgt != rt || W->Bot != bm)
	{
	  SetRefresh(    lt, W->Top,     rt,     bm);
	  SetRefreshWind(W);
	}
    }
  W->Cursor.Shape = cursor;
  W->Flags.Repaint = TRUE;
}
