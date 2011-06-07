#include	<base.h>


Boolean SelectPrevWH (Wind * W)
{
  WindHan * wh = W->AuxPtr;
  while (TRUE)
    {
      Wind * w;
      if (wh == WHan) while (wh->Nam != NULL) ++ wh;
      if ((w = (-- wh)->Wnd) != NULL) if (w->Flags.Switchable) break;
    }
  BringToFront(wh->Wnd);
  return TRUE;
}
