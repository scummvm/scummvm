#include	<general.h>

#define		M	((byte far *) m)

word ChkSum (void far * m, word n)
{
  byte b = 0;
  if (n) while (n --) b += *(M++);
  return b;
}
