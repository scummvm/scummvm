#include	<wind.h>


int WindBodyWid (Wind *W)
{
  return ((W == NULL || W->Flags.Zoomed) ? MaxScrWid
					 : (W->Rgt+1 - W->Lft))
	 - 2 * W->Flags.Frame;
}






