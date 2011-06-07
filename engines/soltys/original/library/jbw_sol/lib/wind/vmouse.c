#include	<wind.h>
#include	<dos.h>


Boolean MenuMouse (Wind * W)
{
  int x, y = -1;

  MX = MouseX(); MY = MouseY();
  if (PosToWind(MX, MY) == W)
    while (LastKey == MouseLeft || LastKey == TwiceLeft)
      {
	x = RelX(W, MX); y = RelY(W, MY);
	while (W->Y < y)
	  {
	    if (! MenuDown(W)) break;
	    if (y >= WindBodyHig(W)) break;
	  }
	while (W->Y > y)
	  {
	    if (! MenuUp(W)) break;
	    if (y < 0) break;
	  }
	SetRefreshWind(W); Refresh();
	if (! MousePressed(1)) break;
	Idle();
	delay(50);
	MX = MouseX(); MY = MouseY();
      }
  return (W->Y == y && x >= 0 && x <= WindBodyWid(W)-1);
}
