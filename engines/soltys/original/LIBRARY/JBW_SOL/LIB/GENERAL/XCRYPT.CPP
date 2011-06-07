#include	<general.h>

#define		BUF	((byte far *) buf)


word XCrypt (void far * buf, word siz, word seed)
{
  word i;
  for (i = 0; i < siz; i ++) * (BUF ++) ^= seed;
  return seed;
}
