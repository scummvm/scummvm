#include	<wind.h>





void ShowBigClock (Wind *w, int x, int y)
{
  static byte *FONT[] = { "ÛßÛßÛ ßßÛßßÛÛ ÛÛßßÛßßßßÛÛßÛÛßÛ   ",
			  "Û Û Û Ûßß ßÛßßÛßßÛÛßÛ  ÛÛßÛßßÛ   ",
			  "ßßßßßßßßßßßß  ßßßßßßß  ßßßßßßß   "  },
	       *COLON =  "ÜÜ ";
  static int old_mm = -1;
  int d, i, j, k, l;
  dword dw = GetTime();
  int hh = (int) dw;
  int mm = (int) (dw >> 16);
  Boolean blank;
  char line[18];

  if (mm == old_mm) return; else old_mm = mm;
  blank = mm < 0; mm &= 0x7FFF;
  for (l = 0; l < 3; l ++)
    {
      k = 0;
      for (i = 0; i < 5; i ++)
	{
	  switch (i)
	    {
	      case 0 : d = hh / 10; if (!d) d = 10; break;
	      case 1 : d = hh % 10; break;
	      case 3 : d = mm / 10; break;
	      case 4 : d = mm % 10; break;
	    }
	  if (i == 2)
	    {
	      line[k++] = (blank) ? ' ' : COLON[l];
	    }
	  else
	    {
	      char *p = FONT[l] + 3 * d;
	      for (j = 0; j < 3; j ++) line[k++] = *(p++);
	    }
	  line[k++] = ' ';
	}
      WriteWindMem(w, x, y+l, line, 18);
    }
}
