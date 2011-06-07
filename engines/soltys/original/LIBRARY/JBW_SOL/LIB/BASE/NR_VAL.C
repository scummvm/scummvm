#include	<base.h>





long NrVal (int f)
{
  return mtol(DbfFldPtr(f), NrRadix, DbfFldLen(f));
}
