#include	"dbf_inc.h"





long DbfVal (int fld)
{
  register int l;
  register char *s;
  int c;
  Boolean neg = FALSE;
  long n = 0;

  for (l = DbfFldLen(fld), s = DbfFldPtr(fld); l; l --, s ++)
    {
      if ((c = *s) == '-') { neg = TRUE; continue; }
      //---$$$$$$$$$$$$$$$$$$$$$$ if (c == ' ') c = '0'; else
      if (! IsDigit(c)) continue;
      n = 10 * n + (c - '0');
    }
  return (neg) ? -n : n;
}
