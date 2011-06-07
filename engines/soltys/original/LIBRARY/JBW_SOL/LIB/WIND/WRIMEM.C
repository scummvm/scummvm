#include	<wind.h>
#include	<stdlib.h>
#include	<mem.h>






void WriteWindMem (Wind *W, int x, int y, const char *txt, int len)
{
  if (W != NULL)
    {
      int wid = W->Wid;
      len = min(len, wid - x);
      memcpy(Image(W) + wid * y + x, txt, len);
      if (W->Flags.Appear)
	{
	  x = AbsX(W, x);
	  y = AbsY(W, y);
	  SetRefresh(x, y, x+len-1, y);
	}
    }
}
