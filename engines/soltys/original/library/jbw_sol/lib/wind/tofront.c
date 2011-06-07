#include	<wind.h>



extern	Wind	*WindStack;





void BringToFront (Wind *W)
{
  Wind *w = WindStack;
  if (W == w) return;
  while (w->Next != W) w = w->Next;
  w->Next = W->Next;
  W->Next = WindStack;
  WindStack = W;
  W = W->Next;
  SetRefreshWind(W);
  SetRefreshWind(WindStack);
}
