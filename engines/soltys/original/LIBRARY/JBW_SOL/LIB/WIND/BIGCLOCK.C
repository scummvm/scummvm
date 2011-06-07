#include	<wind.h>



extern	Wind	*BigClockWind = NULL;




void BigClock (Boolean appear)
{
  if (appear)
    {
      if (BigClockWind == NULL)
	{
	  BigClockWind = MakeBigClock(0, 0, HLP);
	  if (BigClockWind == NULL) return;
	  MoveWind(BigClockWind, MaxScrWid-WindWid(BigClockWind), ZoomTop+1);
	}
      ShowBigClock(BigClockWind, 1, 0);
    }
  else
    {
      if (BigClockWind != NULL)
	{
	  CloseWind(BigClockWind);
	  BigClockWind = NULL;
	}
    }
}
