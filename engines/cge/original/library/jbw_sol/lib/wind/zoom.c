#include	<wind.h>


Boolean ZoomWind (Wind *W)
{
  if (W == NULL || !W->Flags.Zoomable) return FALSE;
  SetRefreshWind(W);
  W->Flags.Zoomed ^= 1;
  return (W->Flags.Repaint = TRUE);
}
