#include	<wind.h>
#include	<string.h>
#include	<stdlib.h>



extern	Wind *WindStack;





void Message (ColorBank c, const char *m)
{
  Wind *w = MakeWind(0, 0, strlen(m) + 3, 4, c);

  if (w == NULL) return;
  SetRefreshWind(WindStack);
  w->Flags.Movable = TRUE;
  StdMove(w);
  WriteWindText(w, 1, 1, m);
  ShowWind(w);
  Refresh();
  CloseWind(w);
}
