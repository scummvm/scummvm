#include	<wind.h>





void MoveWindHorz (Wind *W, int x)
{
  int dx = ((x < 0) ? ((MaxScrWid-(W->Rgt+1-W->Lft))/2) : x) - W->Lft;
  SetRefreshWind(W);
  W->Lft += dx;
  W->Rgt += dx;
  SetRefreshWind(W);
}
