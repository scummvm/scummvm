#include	<wind.h>
#include	<stdlib.h>
#include	<mem.h>



extern char (*Convr)(char);





void WriteWindMemConvr (Wind *W, int x, int y, const char *txt, int len)
{
  if (W != NULL)
    {
      int wid = W->Wid;
      char * p;
      len = min(len, wid - x);
      p = Image(W) + wid * y + x;

      if (Convr)
	{
	  char * q = p + len;
	  while (p != q) * (p ++) = Convr(* (txt ++));
	}
      else memcpy(p, txt, len);

      if (W->Flags.Appear)
	{
	  x = AbsX(W, x);
	  y = AbsY(W, y);
	  SetRefresh(x, y, x+len-1, y);
	}
    }
}
