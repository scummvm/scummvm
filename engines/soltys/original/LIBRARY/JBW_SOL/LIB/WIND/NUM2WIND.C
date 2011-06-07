#include	<wind.h>



extern	Wind *		WindStack;


Wind *NumToWind (int num)
{
  Wind *w = WindStack;
  while (w != NULL && num --) w = w->Next;
  return w;
}





