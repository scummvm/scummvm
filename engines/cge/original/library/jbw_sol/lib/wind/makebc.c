#include	<wind.h>





Wind * MakeBigClock (int x, int y, ColorBank c)
{
  Wind *w = MakeWind(x, y, x+20, y+4, c);
  if (w != NULL)
    {
      w->Flags.Movable = TRUE;
      ShowWind(w);
    }
  return w;
}
