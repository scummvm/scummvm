#include	<wind.h>





void MoveWindVert (Wind *W, int y)
{
  int dy = ((y < 0) ? ((MaxScrHig-(W->Bot+1-W->Top))/2) : y) - W->Top;
  SetRefreshWind(W);
  W->Top += dy;
  W->Bot += dy;
  SetRefreshWind(W);
}
