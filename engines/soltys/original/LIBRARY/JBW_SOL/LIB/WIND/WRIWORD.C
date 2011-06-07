#include	<wind.h>






void WriteWindWord (Wind *W, int x, int y, word n)
{
  int i, w = W->Wid;
  char *s = Image(W) + w * y + x;

  for (i = 4; i >= 0; i --)
    {
      s[i] = (n != 0 || i == 4) ? ('0' + n % 10) : ' ';
      n /= 10;
    }
  if (W->Flags.Appear)
    {
      x = AbsX(W, x);
      y = AbsY(W, y);
      SetRefresh(x, y, x+4, y);
    }
}
