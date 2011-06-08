#include	<wind.h>

#define		wid	11




void WriteWindLong (Wind *W, int x, int y, long n)
{
  int i, w = W->Wid;
  char *s = Image(W) + w * y + x;
  Boolean neg = n < 0;

  if (neg) n = -n;
  for (i = wid-1; i >= 0; i --)
    {
      if (n != 0 || i == wid-1) s[i] = '0' + n % 10;
      else
	if (neg)
	  {
	    s[i] = '-';
	    neg = FALSE;
	  }
	else s[i] = ' ';
      n /= 10;
    }
  if (W->Flags.Appear)
    {
      x = AbsX(W, x);
      y = AbsY(W, y);
      SetRefresh(x, y, x+wid-1, y);
    }
}
