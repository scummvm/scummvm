#include	<wind.h>





void MoveWind (Wind *W, int x, int y)
{
  MoveWindHorz(W, x);
  MoveWindVert(W, y);
}
