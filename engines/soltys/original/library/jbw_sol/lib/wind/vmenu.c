#include	<wind.h>
#include	<stdlib.h>






Boolean MenuUp (Wind *W)
{
  if (W->Vp == 0) return FALSE;
  -- W->Vp;
  if (W->Y) -- W->Y;
  SetRefreshWind(W);
  return TRUE;
}




Boolean MenuDown (Wind *W)
{
  if (W->Vp >= W->Hig-1) return FALSE;
  ++ W->Vp;
  if (W->Y < WindBodyHig(W)-1) ++ W->Y;
  SetRefreshWind(W);
  return TRUE;
}




Boolean MenuPgUp (Wind *W)
{
  W->Vp -= WindBodyHig(W); W->Vp = max(W->Vp, 0);
  W->Y = min(W->Y, W->Vp);
  SetRefreshWind(W);
  return TRUE;
}





Boolean MenuPgDn (Wind *W)
{
  int wh = WindBodyHig(W);
  W->Vp += wh; W->Vp = min(W->Vp, W->Hig-1);
  W->Y = max(W->Y, W->Vp+wh-W->Hig);
  W->Y = min(W->Y, W->Vp);
  SetRefreshWind(W);
  return TRUE;
}





Boolean MenuHome (Wind *W)
{
  W->Vp = (W->Y = 0);
  SetRefreshWind(W);
  return TRUE;
}






Boolean MenuEnd (Wind *W)
{
  int last = WindBodyHig(W)-1;
  W->Vp = W->Hig-1; W->Y = min(last, W->Vp);
  SetRefreshWind(W);
  return TRUE;
}





void RepaintMenuWind (Wind *w)
{
  while (w->Y >= WindBodyHig(w))
    {
      MenuUp(w);
      MenuDown(w);
    }
  SetRefreshWind(w);
}
