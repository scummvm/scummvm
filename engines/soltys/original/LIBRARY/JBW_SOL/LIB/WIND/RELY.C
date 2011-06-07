#include	<wind.h>


int RelY (Wind *W, int y)
{
  return y
       - ((W == NULL || W->Flags.Zoomed) ? ZoomTop : W->Top)
       - (W->Flags.Frame);
}





