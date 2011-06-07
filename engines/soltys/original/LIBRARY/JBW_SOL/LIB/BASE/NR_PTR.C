#include	<base.h>


char * BaseNrPtr (int B)
{
  char * np;
  BasePush(B);
  np = DbfFldPtr(0);
  BasePop();
  return np;
}
