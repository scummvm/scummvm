#include	<wind.h>
#include	<stddef.h>


extern	Wind *		WindStack;


word ShowFieldChar (void)
{
  asm mov AH,[BX].Color+FLD_N
  asm mov AL,[BX].(Wind)X
  return _AX;
}




Wind * MakeField (int l, int t, int r, int b, ColorBank c, char d)
{
  if (CreateWind(l, t, r, b, c, FALSE, FALSE) == NULL) return NULL;
  WindStack->X = d;
  WindStack->ShowProc = ShowFieldChar;
  return WindStack;
}



