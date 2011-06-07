#include	<wind.h>
#include	<mem.h>


int UpperPL (int c)
{
  if (c < 256 && c >= 'a')
    {
      if (c <= 'z') return c & ~0x20;
      else
	{
	  char *p = memchr(Male, c, 9);
	  if (p != NULL) return Duze[p - Male];
	}
    }
  return c;
}
