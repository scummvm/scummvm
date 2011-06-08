#include        <wind.h>



char *lProgress (long now, long tot, int len)
{
  now = (now * len) / tot;
  return Progress((int)now, len, len);
}
