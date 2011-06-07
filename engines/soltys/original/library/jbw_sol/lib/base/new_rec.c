#include	<base.h>







void NewRec (int B)
{
  long n;
  int ord;

  BasePush(B);
  ord = DbfOrder();

  DbfLock();
  DbfGoBottom();
  SetOrder(0);
  DbfSkip(-1);
  SetOrder(ord);
  n = NrVal(0);

  DbfGoBottom();
  NrStr(0, n+1);
  DbfUnlock();
  BasePop();
}
