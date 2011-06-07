#include        <wind.h>
#include        <string.h>


char *Stuff (char *s1, int p, int n, const char *s2)
{
  char s[256];
  strcpy(s, s1+p+n);
  strcpy(s1+p, s2);
  return strcat(s1, s);
}
