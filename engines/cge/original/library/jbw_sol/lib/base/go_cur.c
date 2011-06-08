#include	<base.h>
#include	<string.h>
#include	<stdlib.h>


void GoCurrent (int B)
{
  BaseHan * bh = &Base[B];

  BasePush(B);
  if (bh->Current < 0)
    {
      DbfGoTop();
      bh->Current = NrVal(0);
    }
  else
    {
      char nr[MaxKeyLen+1];
      int ord = DbfOrder(), len = bh->Format[0].Len;

      ltom(bh->Current, nr, NrRadix, len);
      nr[len] = '\0';

      SetOrder(0);
      if (DbfIsIndexed())
	{
	  RecPos rn = DbfRecNo();
	  DbfFind(nr);
	  DbfSkip(0);
	  if (rn != DbfRecNo()) BaseSkip(B);
	}
      SetOrder(ord);
    }
  BasePop();
}
