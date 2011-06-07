#include	<wind.h>
#include	<string.h>
#include	<ctype.h>





Wind * FormatMenu (Wind *W)
{
  char *m = Image(W);
  int w = W->Wid,
      h = W->Hig;
  byte used[256];
  int i, n, c;
  Boolean u;

  memset(used, 0, sizeof(used));

  for (n = 0; n < h; n ++)
    {
      u = FALSE;
      for (i = 0; i < w; i ++)
	{
	  c = tolower(*m);
	  if (!u && IsLower(c))
	    {
	      if (! used[c])
		{
		  used[c] = TRUE;
		  c = toupper(c);
		  u = TRUE;
		}
	    }
	  *m = c;
	  ++ m;
	}
    }
  W->Flags.KeyChoice = TRUE;
  return W;
}
