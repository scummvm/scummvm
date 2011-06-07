#include	<base.h>





void NrStr (int f, long n)
{
  ltom(n, DbfFldPtr(f), NrRadix, DbfFldLen(f));
}
