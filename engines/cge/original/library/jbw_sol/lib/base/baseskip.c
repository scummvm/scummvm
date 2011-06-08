#include	<base.h>






//---- set repaint flags in database windows
void BaseSkip (int B)
{
  BaseHan * bh = &Base[B];
  WindHan * wh;

  BasePush(B);
  DbfSkip(0);
  if (DbfEof()) DbfSkip(-1);
  bh->Current = NrVal(0);
  for (wh = WHan; wh->Nam != NULL; wh ++)
    if (((wh->Sel == B && wh->Tpe != TXT) || wh->Lnk == B) && wh->Wnd != NULL)
      {
	Wind * w = wh->Wnd;
	if (wh->Tpe == TXT) WES(w)->Valid = FALSE;
	w->Flags.Repaint = TRUE;
      }
//  HoldOff(); $$$$$$$$$$$$$$$$$$$$$$$$$$$$
  BasePop();
}
