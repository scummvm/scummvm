#include	<base.h>


Boolean SelectNextWH (Wind * W)
{
  WindHan * wh = W->AuxPtr;
  while (TRUE)
    {
      Wind * w;
      if ((++ wh)->Nam == NULL) wh = WHan;
      if ((w = wh->Wnd) != NULL) if (w->Flags.Switchable) break;
    }
  BringToFront(wh->Wnd);
  return TRUE;
}
