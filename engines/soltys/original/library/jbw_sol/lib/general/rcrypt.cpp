#include	<general.h>


#define		BUF	((byte far *) buf)


word RCrypt (void far * buf, word siz, word seed)
{
  if (buf && siz)
    {
      byte far * q = BUF + (siz-1);
      seed = FastRand(seed);
      * (BUF ++) ^= seed;
      while (buf < q) * (BUF ++) ^= FastRand();
      if (buf == q) * BUF ^= (seed = FastRand());
    }
  return seed;
}
