#include	<wind.h>


int WindWid (Wind *W)
{
  return (W == NULL || W->Flags.Zoomed) ? MaxScrWid : (W->Rgt-W->Lft+1);
}






