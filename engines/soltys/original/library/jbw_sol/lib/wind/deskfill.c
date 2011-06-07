#include	<wind.h>


extern	Wind *	Desk;


void SetDeskFill (char fill)
{
  if (Desk) Desk->X = fill;
  SetRefreshWind(Desk);
}
