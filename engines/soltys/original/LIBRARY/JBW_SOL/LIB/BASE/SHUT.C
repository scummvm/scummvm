#include	<base.h>




int BaseWindCount (int B)
{
  WindHan * wh;
  int n = 0;

  for (wh = WHan; wh->Nam != NULL; wh ++)
    if (wh->Sel == B && wh->Wnd != NULL)
       ++ n;
  return n;
}




void ShutSlaves (int B)
{
  WindHan * wh;

  for (wh = WHan; wh->Nam != NULL; wh ++)
    if (wh->Lnk == B && wh->Wnd != NULL)
       CloseWH(wh->Wnd);
}





void EditShut (Wind *W)
{
  WindHan *wh = W->AuxPtr;
  int B = wh->Sel;

  if (BaseWindCount(B) == 1)
    {
      int sel = DbfSelected();
      W->Flags.EditEnable = FALSE;
//      W->ReptProc(W);
      DbfSelect(wh->Sel);
      DbfClose();
      DbfSelect(sel);
      ShutSlaves(B);
    }
  else BaseSkip(B);
}
