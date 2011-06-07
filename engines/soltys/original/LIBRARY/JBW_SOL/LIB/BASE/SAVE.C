#include	<base.h>


Boolean BaseSave (Wind * W)
{
  WindHan *wh = W->AuxPtr;

  DbfCommit();
  BaseSkip(wh->Sel);
  return TRUE;
}
