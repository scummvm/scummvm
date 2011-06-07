#include	<wind.h>


int RelX (Wind *W, int x)
{
  return ((W == NULL || W->Flags.Zoomed) ? x : (x - W->Lft))
       - (W->Flags.Frame);
}
