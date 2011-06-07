#include	<wind.h>
#include	<alloc.h>





word *SaveScreen (void)
{
  word *ssav;
  int i;

  GetScrSiz(); Screen = ScrAdr(0, 0);
  if ((ssav = malloc((MaxScrHig * MaxScrWid + 2) * sizeof(*ssav))) != NULL)
    {
      for (i = 0; i < MaxScrHig * MaxScrWid; i ++) ssav[i] = Screen[i];
      ssav[i] = WhereX; ssav[i+1] = WhereY;
    }
  return ssav;
}





void RestoreScreen (word *ssav)
{
  int i;

  for (i = 0; i < MaxScrHig * MaxScrWid; i ++) Screen[i] = ssav[i];
  SetXY(ssav[i], ssav[i+1]);
  free(ssav);
}
