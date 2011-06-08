#include	<wind.h>



void SetRefreshChar (Wind *W, int x, int y)
{
  x = AbsX(W, x);
  y = AbsY(W, y);
  SetRefresh(x, y, x, y);
}






