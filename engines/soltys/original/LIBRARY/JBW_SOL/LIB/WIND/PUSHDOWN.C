#include	<wind.h>
#include	<stddef.h>



extern	Wind	*WindStack;





void PushDown (Wind *W)
{
  if (W->Next != NULL)
    {
      SetRefreshWind(W);
      if (W == WindStack)
	{
	  WindStack = W->Next;
	  W->Next = WindStack->Next;
	  WindStack->Next = W;
	  SetRefreshWind(WindStack);
	}
      else
	{
	  Wind *w = WindStack;
	  while (w->Next != W) w = w->Next;
	  w->Next = W->Next;
	  w = W->Next;
	  W->Next = w->Next;
	  w->Next = W;
	}
    }
}
