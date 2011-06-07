#include	<wind.h>


extern	void    (*IdleProc)	(void);

void DragWind (Wind *W)
{
  int mx = MX, my = MY, dx, dy, l, t, r, b;
  word cursor = W->Cursor.Shape;

  W->Cursor.Shape = OFF_Cursor;
  while (W->Flags.Movable && MousePressed(1))
    {
      if (IdleProc != NULL) IdleProc();
      Refresh();
      MX = MouseX();
      MY = MouseY();
      l = W->Lft,
      t = W->Top,
      r = W->Rgt;
      b = W->Bot;
      dx = MX - mx; dy = MY - my;
      if (l+dx < 0) dx -= l+dx;
      if (t+dy < ZoomTop) dy -= t+dy-1;
      if (r+dx > MaxScrWid-1) dx -= r+dx-(MaxScrWid-1);
      if (b+dy > ZoomBot) dy -= b+dy-ZoomBot;
      if (dx | dy)
	{
	  SetRefreshWind(W);
	  W->Lft += dx;
	  W->Top += dy;
	  W->Rgt += dx;
	  W->Bot += dy;
	  SetRefreshWind(W);
	  mx = MX; my = MY;
	}
    }
  W->Cursor.Shape = cursor;
}




