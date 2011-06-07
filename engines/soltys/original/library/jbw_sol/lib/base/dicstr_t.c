#include	<base.h>
#include	<string.h>




extern	char	BaseBuff[256];



char * DictStrTrim (int cpl)
{
  int B = cpl >> 8;
  int fld = cpl & 0xFF;
  int ord;
  int n = DbfFldLen(fld);

  memcpy(BaseBuff, DbfFldPtr(fld), n);
  BaseBuff[n] = '\0';
  BasePush(B);
  ord = DbfOrder();
  SetOrder(0);		// reference number
  if (DbfFind(BaseBuff)) DbfString(1, BaseBuff);
  else BaseBuff[0] = '\0';
  SetOrder(ord);
  BasePop();

  return BaseBuff;
}
