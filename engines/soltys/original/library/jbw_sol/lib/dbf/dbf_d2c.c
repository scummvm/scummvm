#include	"dbf_inc.h"



extern	char	DateConvr[DCONVR_MAX+1] = "1234-56-78";



char *DbfD2C (const char *d)
{
  static char s[DCONVR_MAX+1];
  char *p = s, *q = DateConvr, c;

  while ((c = *q) != '\0')
    {
      if (c >= '1' && c <= '8') *p = d[c - '1'];
      else *p = c;
      ++ q;
      ++ p;
    }
  *p = c;
  return s;
}





/*
char *LFill (const char *s, int c, int l)
{
  static char b[256]; int ls = strlen(s);
  l = min(l, 255); ls = min(ls, l);
  memset(b, c, l-ls); strcpy(b+l-ls, s); return b;
}
*/





/*
char *RFill (const char *s, int c, int l)
{
  static char b[256]; int ls = strlen(s);
  l = min(l, 255); ls = min(ls, l);
  strcpy(b, s); memset(b+ls, c, l-ls);
  b[l] = '\0'; return b;
}
*/