#include	<general.h>

#define		BUF	((byte far *) buf)


word RXCrypt (void far * buf, word siz, word seed)
{
  word i;
  seed = FastRand(seed);
  for (i = 0; i < siz; i ++) * (BUF ++) ^= seed;
  return seed;
}
