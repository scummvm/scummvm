#include	<wind.h>



extern	int	StdX = -1, StdY = -1;




void StdMove (Wind *w)
{
  MoveWindHorz(w, StdX);
  MoveWindVert(w, StdY);
}
