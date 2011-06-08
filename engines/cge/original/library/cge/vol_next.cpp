#include	"vol.h"
#include	<string.h>


const char * VFILE::Next (void)
{
  static char n[MAXFILE-1+MAXEXT];
  BT_KEYPACK far * btp = Cat.Next();
  if (btp == NULL) return NULL;
  _fstrcpy(n, btp->Key);
  return n;
}
