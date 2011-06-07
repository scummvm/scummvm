#include	<wind.h>




word ShowPureChar (void)
{
  asm add  SI,[BX].Body.Near
  asm mov  AL,[SI]
  asm mov  AH,[BX].Color+FLD_N
  return _AX;
}
