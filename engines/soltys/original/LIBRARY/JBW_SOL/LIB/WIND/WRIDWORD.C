#include	<wind.h>






void WriteWindDword (Wind *W, int x, int y, dword n)
{
  int i, w = W->Wid;
  char *s = Image(W) + w * y + x;

  for (i = 9; i >= 0; i --)
    {
      s[i] = (n != 0 || i == 9) ? ('0' + n % 10) : ' ';
      n /= 10;
    }
  if (W->Flags.Appear)
    {
      x = AbsX(W, x);
      y = AbsY(W, y);
      SetRefresh(x, y, x+9, y);
    }
}
