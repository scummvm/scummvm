#include        <wind.h>


char *lWhereIs (long now, long tot, int len)
{
  now = (len * now) / tot;
  return WhereIs((int)now, len, len);
}
