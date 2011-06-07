#include	<wind.h>


int WindHig (Wind *W)
{
  return (W == NULL || W->Flags.Zoomed) ? (ZoomBot-ZoomTop+1) : (W->Bot-W->Top+1);
}






