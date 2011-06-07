#include	<wind.h>


int WindBodyHig (Wind *W)
{
  return ((W == NULL || W->Flags.Zoomed) ? (ZoomBot-ZoomTop+1)
					 : (W->Bot+1 - W->Top))
	 - 2 * W->Flags.Frame;
}






